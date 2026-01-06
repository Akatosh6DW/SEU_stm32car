// // #include "tracking.h"
// // #include "encoder.h" // 为了修改 Target_Speed
// // #include "gpio.h"

// // static int16_t Last_Error = 0;

// // uint8_t Track_Read(void)
// // {
// //     uint8_t sensor_val = 0;
// //     // 使用 main.h 的宏
// //     if (HAL_GPIO_ReadPin(TRACK0_GPIO_Port, TRACK0_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 0);
// //     if (HAL_GPIO_ReadPin(TRACK1_GPIO_Port, TRACK1_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 1);
// //     if (HAL_GPIO_ReadPin(TRACK2_GPIO_Port, TRACK2_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 2);
// //     if (HAL_GPIO_ReadPin(TRACK3_GPIO_Port, TRACK3_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 3);
// //     if (HAL_GPIO_ReadPin(TRACK4_GPIO_Port, TRACK4_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 4);
// //     if (HAL_GPIO_ReadPin(TRACK5_GPIO_Port, TRACK5_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 5);
// //     if (HAL_GPIO_ReadPin(TRACK6_GPIO_Port, TRACK6_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 6);
// //     if (HAL_GPIO_ReadPin(TRACK7_GPIO_Port, TRACK7_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 7);
// //     return sensor_val;
// // }

// // int16_t Get_Error(uint8_t sensor_state)
// // {
// //     if (sensor_state == 0) return Last_Error;
// //     int32_t error_sum = 0;
// //     int count = 0;
    
// //     // 左侧 (负分)
// //    // if (sensor_state & (1 << 7)) { error_sum -= 40; count++; }
// //     if (sensor_state & (1 << 7)) { error_sum -= 50; count++; }
// //     if (sensor_state & (1 << 6)) { error_sum -= 40; count++; }
// //     if (sensor_state & (1 << 5)) { error_sum -= 20; count++; }
// //     if (sensor_state & (1 << 4)) { error_sum -= 10; count++; }
// //     // 右侧 (正分)
// //     if (sensor_state & (1 << 3)) { error_sum += 10; count++; }
// //     if (sensor_state & (1 << 2)) { error_sum += 20; count++; }
// //     if (sensor_state & (1 << 1)) { error_sum += 40; count++; }
// //     if (sensor_state & (1 << 0)) { error_sum += 50; count++; }

// //     if (count == 0) return Last_Error;
// //     return (int16_t)(error_sum / count);
// // }

// // void Tracking_Handler(void)
// // {
// //     // 1. 读取传感器 & 计算位置误差
// //     uint8_t current_sensor = Track_Read();
// //     int16_t error = Get_Error(current_sensor);

// //     // 2. 循迹 PID 计算 (输出 = 左右轮的目标速度差)
// //     float speed_diff = (TRACK_KP * error) + (TRACK_KD * (error - Last_Error));
// //     Last_Error = error;

// //     // 3. 设定左右轮的目标速度 (不是 PWM!)
// //     // 基础速度 20，如果偏左(error负)，speed_diff负
// //     // Target_L = 20 + (-5) = 15 (减速)
// //     // Target_R = 20 - (-5) = 25 (加速) -> 向左修正
// //     float target_L = TRACK_BASE_SPEED - speed_diff;
// //     float target_R = TRACK_BASE_SPEED + speed_diff;

// //     // 4. 限幅 (防止倒车或超速)
// //     if (target_L < 0) target_L = 0;
// //     if (target_L > 90) target_L = 90; // 假设最大跑90cm/s
    
// //     if (target_R < 0) target_R = 0;
// //     if (target_R > 90) target_R = 90;

// //     // 5. 【关键】更新全局变量，让 encoder.c 去执行
// //     Target_Speed_L = target_L;
// //     Target_Speed_R = target_R;
    
// // }
// #include "tracking.h"
// #include "encoder.h" 
// #include "gpio.h"
// #include "stm32f4xx_hal.h"
// #include <math.h>

// static int16_t Last_Error = 0;

// // 定义特殊标志位值 (远超正常误差范围 -50~50)
// #define ERR_LEFT_TURN   -1000
// #define ERR_RIGHT_TURN   1000

// uint8_t Track_Read(void)
// {
//     uint8_t sensor_val = 0;
//     // 假设 BIT7 是最左边，BIT0 是最右边
//     if (HAL_GPIO_ReadPin(TRACK0_GPIO_Port, TRACK0_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 0);
//     if (HAL_GPIO_ReadPin(TRACK1_GPIO_Port, TRACK1_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 1);
//     if (HAL_GPIO_ReadPin(TRACK2_GPIO_Port, TRACK2_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 2);
//     if (HAL_GPIO_ReadPin(TRACK3_GPIO_Port, TRACK3_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 3);
//     if (HAL_GPIO_ReadPin(TRACK4_GPIO_Port, TRACK4_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 4);
//     if (HAL_GPIO_ReadPin(TRACK5_GPIO_Port, TRACK5_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 5);
//     if (HAL_GPIO_ReadPin(TRACK6_GPIO_Port, TRACK6_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 6);
//     if (HAL_GPIO_ReadPin(TRACK7_GPIO_Port, TRACK7_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 7);
//     return sensor_val;
// }

// int16_t Get_Error(uint8_t sensor_state)
// {
//     // 1. 如果全灭，保持上一次状态 (盲跑)
//     if (sensor_state == 0) return Last_Error;

//     // ============================================================
//     // 【特征匹配区】 直接识别二进制模式
//     // ============================================================

//     // --- 十字路口 (Crossroad) ---
//     // 特征：中间6个以上全黑，或者全黑 (0xFF, 0x7E, 0xBD等)
//     // 掩码判断：如果中间6位都是1 (0x7E = 01111110)，认为是十字路口
//     // 动作：当作直线处理，Error = 0
//     if ((sensor_state & 0x7E) == 0x7E) 
//     {
//         return 0; 
//     }

//     // --- 左直角 (Left 90) ---
//     // 特征：左边4个全黑 (1111xxxx -> 0xF0)
//     // 判断：(val & 0xF0) == 0xF0 意味着左边4个必须全是1，右边无所谓
//     if ((sensor_state & 0xF0) == 0xF0)
//     {
//         return ERR_LEFT_TURN; // 返回 -1000
//     }
    
    
//     // // --- 右直角 (Right 90) ---
//     // // 特征：右边4个全黑 (xxxx1111 -> 0x0F)

//     if ((sensor_state & 0x0F) == 0x0F)
//     {
//         return ERR_RIGHT_TURN; // 返回 1000
//     }

//     // ============================================================
//     // 【普通加权平均区】 普通循迹逻辑
//     // ============================================================
//     int32_t error_sum = 0;
//     int count = 0;
    
//     // 左侧权重
//     if (sensor_state & (1 << 7)) { error_sum -= 50; count++; }
//     if (sensor_state & (1 << 6)) { error_sum -= 40; count++; }
//     if (sensor_state & (1 << 5)) { error_sum -= 20; count++; }
//     if (sensor_state & (1 << 4)) { error_sum -= 10; count++; }
//     // 右侧权重
//     if (sensor_state & (1 << 3)) { error_sum += 10; count++; }
//     if (sensor_state & (1 << 2)) { error_sum += 20; count++; }
//     if (sensor_state & (1 << 1)) { error_sum += 40; count++; }
//     if (sensor_state & (1 << 0)) { error_sum += 50; count++; }

//     if (count == 0) return Last_Error;
//     return (int16_t)(error_sum / count);
// }

// void Tracking_Handler(void)
// {
//     // 1. 读取并计算误差 (包含特殊状态判断)
//     uint8_t current_sensor = Track_Read();
//     int16_t error = Get_Error(current_sensor);

//     // ==========================================================
//     // 2. 检查是否是特殊状态 (拦截逻辑)
//     // ==========================================================
    
//     // 设定直角弯的原地旋转速度
//     float turn_speed = 60.0f; 

//     if (error == ERR_LEFT_TURN) 
//     {
//         // === 左直角 ===
//         // 左轮倒车，右轮前进 (原地左旋)
//         Target_Speed_L = -turn_speed; 
//         Target_Speed_R = turn_speed;
//          HAL_Delay(100);
//         return; // 直接返回，不跑 PID
//     }
//     else if (error == ERR_RIGHT_TURN)
//     {
//         // === 右直角 ===
//         // 左轮前进，右轮倒车 (原地右旋)
//         Target_Speed_L = turn_speed;
//         Target_Speed_R = -turn_speed;
//         HAL_Delay(100);
//         return; // 直接返回，不跑 PID
//     }

//     // ==========================================================
//     // 3. 普通 PID 循迹 (只有 error 在 -50~50 之间才走这里)
//     // ==========================================================
    
//     float speed_diff = (TRACK_KP * error) + (TRACK_KD * (error - Last_Error));
//     Last_Error = error;

//     float target_L = TRACK_BASE_SPEED - speed_diff;
//     float target_R = TRACK_BASE_SPEED + speed_diff;

//     // 4. 普通模式限幅 (禁止负数，保证稳)
//     if (target_L < 0) target_L = 0;
//     if (target_L > 90) target_L = 90;
    
//     if (target_R < 0) target_R = 0;
//     if (target_R > 90) target_R = 90;

//     Target_Speed_L = target_L;
//     Target_Speed_R = target_R;
// }
#include "tracking.h"
#include "encoder.h" 
#include "gpio.h"
#include <math.h>

static int16_t Last_Error = 0;

// 定义特殊返回值
#define ERR_LEFT_TURN    -1000
#define ERR_RIGHT_TURN    1000
int total_active=0;

// 辅助函数：数一下有多少个灯亮了
int Count_Set_Bits(uint8_t n) {
    int count = 0;
    while (n > 0) {
        if (n & 1) count++;
        n >>= 1;
    }
    return count;
}

uint8_t Track_Read(void)
{
    // 【完全保留你现在的逻辑】
    // 既然你说这个能跑，那我们就认为：GPIO_PIN_SET (高电平) = 有线
    uint8_t sensor_val = 0;
    if (HAL_GPIO_ReadPin(TRACK0_GPIO_Port, TRACK0_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 0);
    if (HAL_GPIO_ReadPin(TRACK1_GPIO_Port, TRACK1_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 1);
    if (HAL_GPIO_ReadPin(TRACK2_GPIO_Port, TRACK2_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 2);
    if (HAL_GPIO_ReadPin(TRACK3_GPIO_Port, TRACK3_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 3);
    if (HAL_GPIO_ReadPin(TRACK4_GPIO_Port, TRACK4_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 4);
    if (HAL_GPIO_ReadPin(TRACK5_GPIO_Port, TRACK5_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 5);
    if (HAL_GPIO_ReadPin(TRACK6_GPIO_Port, TRACK6_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 6);
    if (HAL_GPIO_ReadPin(TRACK7_GPIO_Port, TRACK7_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 7);
    return sensor_val;
}

int16_t Get_Error(uint8_t sensor_state)
{
    // 1. 如果全灭，保持惯性
    if (sensor_state == 0) return Last_Error;

    // ============================================================
    // 【新增：直角弯识别区】(基于你现在的传感器逻辑)
    // ============================================================
    
   total_active = Count_Set_Bits(sensor_state);

    // 只有当亮起的灯足够多 (>=5个) 时，才怀疑是直角弯或十字路口
    // 这样普通巡线 (亮1-3个) 绝不会误触发！
    if (total_active >= 5)
    {
        // 1. 先排除十字路口 (>=7个灯亮) -> 认为是直线
        if (total_active >= 7) {
            return 0; 
        }

        // 2. 统计左边(Bit7-4) 和 右边(Bit3-0) 谁亮的多
        int left_cnt = Count_Set_Bits(sensor_state & 0xF0);
        int right_cnt = Count_Set_Bits(sensor_state & 0x0F);

        // 3. 判断方向
        // 如果左边比右边多亮至少 2 个灯 (例如左边4个全亮，右边只亮1-2个) -> 左直角
        if (left_cnt >= right_cnt + 2) {
            return ERR_LEFT_TURN;
        }
        // 如果右边比左边多亮至少 2 个灯 -> 右直角
        if (right_cnt >= left_cnt + 2) {
            return ERR_RIGHT_TURN;
        }
    }

    // ============================================================
    // 【保留：你原本的 PID 权重计算】
    // ============================================================
    int32_t error_sum = 0;
    int count = 0;
    
    // 左侧权重
    if (sensor_state & (1 << 7)) { error_sum -= 50; count++; }
    if (sensor_state & (1 << 6)) { error_sum -= 40; count++; }
    if (sensor_state & (1 << 5)) { error_sum -= 20; count++; }
    if (sensor_state & (1 << 4)) { error_sum -= 10; count++; }
    // 右侧权重
    if (sensor_state & (1 << 3)) { error_sum += 10; count++; }
    if (sensor_state & (1 << 2)) { error_sum += 20; count++; }
    if (sensor_state & (1 << 1)) { error_sum += 40; count++; }
    if (sensor_state & (1 << 0)) { error_sum += 50; count++; }

    if (count == 0) return Last_Error;
    return (int16_t)(error_sum / count);
}

void Tracking_Handler(void)
{
    // 1. 读取并计算
    uint8_t current_sensor = Track_Read();
    int16_t error = Get_Error(current_sensor);

    // ==========================================================
    // 2. 拦截直角弯 (特种兵逻辑)
    // ==========================================================
    
    float turn_speed = 60.0f; // 这里的速度你可以根据实际情况调

    if (error == ERR_LEFT_TURN) 
    {
        // === 左直角 ===
        // 根据你之前的反馈：左轮退，右轮进
        Target_Speed_L = -turn_speed; 
        Target_Speed_R = turn_speed;
        return; // 直接返回，跳过后面的 PID
    }
    else if (error == ERR_RIGHT_TURN)
    {
        // === 右直角 ===
        // 左轮进，右轮退
        Target_Speed_L = turn_speed;
        Target_Speed_R = -turn_speed;
        return; // 直接返回
    }

    // ==========================================================
    // 3. 普通 PID 循迹 (原封不动保留你觉得稳的逻辑)
    // ==========================================================
    
    float speed_diff = (TRACK_KP * error) + (TRACK_KD * (error - Last_Error));
    Last_Error = error;

    float target_L = TRACK_BASE_SPEED - speed_diff;
    float target_R = TRACK_BASE_SPEED + speed_diff;

    // 4. 普通限幅 (禁止负数)
    if (target_L < 0) target_L = 0;
    if (target_L > 90) target_L = 90;
    
    if (target_R < 0) target_R = 0;
    if (target_R > 90) target_R = 90;

    Target_Speed_L = target_L;
    Target_Speed_R = target_R;
}