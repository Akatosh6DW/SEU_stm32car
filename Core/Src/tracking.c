// // #include "tracking.h"
// // #include "encoder.h" 
// // #include "gpio.h"
// // #include "stm32f4xx_hal.h"
// // #include <math.h>

// // // 全局变量用于调试
// // uint8_t sensor_value = 0;
// // static int16_t Last_Error = 0;

// // // 定义特殊返回值
// // #define ERR_LEFT_TURN    -1000
// // #define ERR_RIGHT_TURN    1000

// // uint8_t Track_Read(void)
// // {
// //     // 【完全保留】你的原版读取函数
// //     uint8_t sensor_val = 0;
// //     if (HAL_GPIO_ReadPin(TRACK0_GPIO_Port, TRACK0_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 0);
// //     if (HAL_GPIO_ReadPin(TRACK1_GPIO_Port, TRACK1_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 1);
// //     if (HAL_GPIO_ReadPin(TRACK2_GPIO_Port, TRACK2_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 2);
// //     if (HAL_GPIO_ReadPin(TRACK3_GPIO_Port, TRACK3_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 3);
// //     if (HAL_GPIO_ReadPin(TRACK4_GPIO_Port, TRACK4_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 4);
// //     if (HAL_GPIO_ReadPin(TRACK5_GPIO_Port, TRACK5_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 5);
// //     if (HAL_GPIO_ReadPin(TRACK6_GPIO_Port, TRACK6_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 6);
// //     if (HAL_GPIO_ReadPin(TRACK7_GPIO_Port, TRACK7_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 7);
    
// //     sensor_value = sensor_val; // 更新全局变量
// //     return sensor_val;
// // }

// // int16_t Get_Error(uint8_t sensor_state)
// // {
// //     // 如果全白 (1111 1111) 或全黑 (0000 0000) 的特殊处理
// //     if (sensor_state == 0xFF) return Last_Error; // 全白：保持
// //     if (sensor_state == 0x00) return 0;          // 全黑：十字路口 -> 视为直行！

// //     // ==========================================================
// //     // 【新增：十字路口拦截器】 (关键修改！)
// //     // ==========================================================
// //     // 逻辑：如果 Bit7(最左) 和 Bit0(最右) 都是 0 (黑)，
// //     // 说明这是一条横线或者十字路口，绝对不是直角弯。
// //     // 0x81 = 1000 0001
// //     // (val & 0x81) == 0 表示两头都是0
// //     if ((sensor_state & 0x81) == 0x00)
// //     {
// //         return 0; // 强制返回0，让PID走直线，防止误判为直角
// //     }

// //     // ==========================================================
// //     // 【绝对匹配区】按照你要求的二进制值 (0=黑, 1=白)
// //     // ==========================================================
    
// //     // --- 左直角判据 ---
// //     // 0000 1111 (0x0F) -> 左边4个黑
// //     // 0000 0111 (0x07) -> 左边3个黑
// //     // 0000 0011 (0x03) -> 左边2个黑
// //     if (sensor_state == 0x0F || sensor_state == 0x07 || sensor_state == 0x03)
// //     {
// //         return ERR_LEFT_TURN; 
// //     }

// //     // --- 右直角判据 ---
// //     // 1111 0000 (0xF0) -> 右边4个黑
// //     // 1110 0000 (0xE0) -> 右边3个黑
// //     // 1100 0000 (0xC0) -> 右边2个黑
// //     if (sensor_state == 0xF0 || sensor_state == 0xE0 || sensor_state == 0xC0)
// //     {
// //         return ERR_RIGHT_TURN; 
// //     }

// //     // ==========================================================
// //     // 【保留：你的原版 PID 权重计算区】
// //     // ==========================================================

// //     int32_t error_sum = 0;
// //     int count = 0;
    
// //     // 左侧 (负分)
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
// //     // 1. 读取 & 计算
// //     uint8_t current_sensor = Track_Read();
// //     int16_t error = Get_Error(current_sensor);

// //     // ==========================================================
// //     // 【拦截执行区】
// //     // ==========================================================
    
// //     float turn_speed = 60.0f; // 坦克掉头速度

// //     if (error == ERR_LEFT_TURN) 
// //     {
// //         // === 左直角 ===
// //         Target_Speed_L = -turn_speed; 
// //         Target_Speed_R = turn_speed;
// //         HAL_Delay(200);
// //         return; 
// //     }
// //     else if (error == ERR_RIGHT_TURN)
// //     {
// //         // === 右直角 ===
// //         Target_Speed_L = turn_speed;
// //         Target_Speed_R = -turn_speed;
// //         HAL_Delay(350);
// //         return; 
// //     }

// //     // ==========================================================
// //     // 【保留：你的原版 PID 执行区】
// //     // ==========================================================

// //     float speed_diff = (TRACK_KP * error) + (TRACK_KD * (error - Last_Error));
// //     Last_Error = error;

// //     float target_L = TRACK_BASE_SPEED - speed_diff;
// //     float target_R = TRACK_BASE_SPEED + speed_diff;

// //     if (target_L < 0) target_L = 0;
// //     if (target_L > 90) target_L = 90; 
    
// //     if (target_R < 0) target_R = 0;
// //     if (target_R > 90) target_R = 90;

// //     Target_Speed_L = target_L;
// //     Target_Speed_R = target_R;
// // }
// #include "tracking.h"
// #include "encoder.h" 
// #include "gpio.h"
// #include "stm32f4xx_hal.h"
// #include <math.h>

// // 全局变量用于调试
// uint8_t sensor_value = 0;
// static int16_t Last_Error = 0;

// // 定义特殊返回值
// #define ERR_LEFT_TURN    -1000
// #define ERR_RIGHT_TURN    1000
// #define ERR_CROSSROAD     5000  // 【新增】十字路口特殊标记

// uint8_t Track_Read(void)
// {
//     // 【完全保留】你的原版读取函数
//     uint8_t sensor_val = 0;
//     if (HAL_GPIO_ReadPin(TRACK0_GPIO_Port, TRACK0_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 0);
//     if (HAL_GPIO_ReadPin(TRACK1_GPIO_Port, TRACK1_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 1);
//     if (HAL_GPIO_ReadPin(TRACK2_GPIO_Port, TRACK2_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 2);
//     if (HAL_GPIO_ReadPin(TRACK3_GPIO_Port, TRACK3_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 3);
//     if (HAL_GPIO_ReadPin(TRACK4_GPIO_Port, TRACK4_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 4);
//     if (HAL_GPIO_ReadPin(TRACK5_GPIO_Port, TRACK5_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 5);
//     if (HAL_GPIO_ReadPin(TRACK6_GPIO_Port, TRACK6_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 6);
//     if (HAL_GPIO_ReadPin(TRACK7_GPIO_Port, TRACK7_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 7);
    
//     sensor_value = sensor_val; // 更新全局变量
//     return sensor_val;
// }

// int16_t Get_Error(uint8_t sensor_state)
// {
//     // 如果全白 (1111 1111) 保持上一次状态
//     if (sensor_state == 0xFF) return Last_Error; 
    
//     // 【修改点 1】 全黑：十字路口 -> 返回特殊值，而不是0
//     if (sensor_state == 0x00) return ERR_CROSSROAD;          

//     // ==========================================================
//     // 【修改点 2】 十字路口拦截器
//     // ==========================================================
//     // 逻辑：如果 Bit7(最左) 和 Bit0(最右) 都是 0 (黑)
//     // 0x81 = 1000 0001
//     // (val & 0x81) == 0 表示两头都是0
//     if ((sensor_state & 0x81) == 0x00)
//     {
//         return ERR_CROSSROAD; // 【改】返回十字路口标记
//     }

//     // ==========================================================
//     // 【绝对匹配区】按照你要求的二进制值 (0=黑, 1=白)
//     // ==========================================================
    
//     // --- 左直角判据 ---
//     // 0000 1111 (0x0F) -> 左边4个黑
//     // 0000 0111 (0x07) -> 左边3个黑
//     // 0000 0011 (0x03) -> 左边2个黑
//     if (sensor_state == 0x0F || sensor_state == 0x07 || sensor_state == 0x03)
//     {
//         return ERR_LEFT_TURN; 
//     }

//     // --- 右直角判据 ---
//     // 1111 0000 (0xF0) -> 右边4个黑
//     // 1110 0000 (0xE0) -> 右边3个黑
//     // 1100 0000 (0xC0) -> 右边2个黑
//     if (sensor_state == 0xF0 || sensor_state == 0xE0 || sensor_state == 0xC0)
//     {
//         return ERR_RIGHT_TURN; 
//     }

//     // ==========================================================
//     // 【保留：你的原版 PID 权重计算区】
//     // ==========================================================

//     int32_t error_sum = 0;
//     int count = 0;
    
//     // 左侧 (负分)
//     if (sensor_state & (1 << 7)) { error_sum -= 50; count++; }
//     if (sensor_state & (1 << 6)) { error_sum -= 40; count++; }
//     if (sensor_state & (1 << 5)) { error_sum -= 20; count++; }
//     if (sensor_state & (1 << 4)) { error_sum -= 10; count++; }
//     // 右侧 (正分)
//     if (sensor_state & (1 << 3)) { error_sum += 10; count++; }
//     if (sensor_state & (1 << 2)) { error_sum += 20; count++; }
//     if (sensor_state & (1 << 1)) { error_sum += 40; count++; }
//     if (sensor_state & (1 << 0)) { error_sum += 50; count++; }

//     if (count == 0) return Last_Error;
//     return (int16_t)(error_sum / count);
// }

// void Tracking_Handler(void)
// {
//     // 1. 读取 & 计算
//     uint8_t current_sensor = Track_Read();
//     int16_t error = Get_Error(current_sensor);

//     // ==========================================================
//     // 【拦截执行区】
//     // ==========================================================
    
//     float turn_speed = 60.0f; // 坦克掉头速度

//     // 【修改点 3】 新增十字路口处理逻辑
//     if (error == ERR_CROSSROAD)
//     {
//         // 遇到十字路口：强制直行
//         Target_Speed_L = TRACK_BASE_SPEED;
//         Target_Speed_R = TRACK_BASE_SPEED;
        
//         // 关键延时！冲过路口，避免出路口时误判直角
//         // 200ms 足以冲过一般的黑胶带宽度
//         HAL_Delay(200); 
//         return; 
//     }

//     if (error == ERR_LEFT_TURN) 
//     {
//         // === 左直角 ===
//         Target_Speed_L = -turn_speed; 
//         Target_Speed_R = turn_speed;
//         HAL_Delay(200);
//         return; 
//     }
//     else if (error == ERR_RIGHT_TURN)
//     {
//         // === 右直角 ===
//         Target_Speed_L = turn_speed;
//         Target_Speed_R = -turn_speed;
//         HAL_Delay(350);
//         return; 
//     }

//     // ==========================================================
//     // 【保留：你的原版 PID 执行区】
//     // ==========================================================

//     float speed_diff = (TRACK_KP * error) + (TRACK_KD * (error - Last_Error));
//     Last_Error = error;

//     float target_L = TRACK_BASE_SPEED - speed_diff;
//     float target_R = TRACK_BASE_SPEED + speed_diff;

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
#include "stm32f4xx_hal.h"
#include <math.h>

// 全局变量
uint8_t sensor_value = 0;
static int16_t Last_Error = 0;

// 定义特殊返回值
#define ERR_LEFT_TURN    -1000
#define ERR_RIGHT_TURN    1000
#define ERR_CROSSROAD     5000 

uint8_t Track_Read(void)
{
    // 【保留原版】
    uint8_t sensor_val = 0;
    if (HAL_GPIO_ReadPin(TRACK0_GPIO_Port, TRACK0_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 0);
    if (HAL_GPIO_ReadPin(TRACK1_GPIO_Port, TRACK1_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 1);
    if (HAL_GPIO_ReadPin(TRACK2_GPIO_Port, TRACK2_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 2);
    if (HAL_GPIO_ReadPin(TRACK3_GPIO_Port, TRACK3_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 3);
    if (HAL_GPIO_ReadPin(TRACK4_GPIO_Port, TRACK4_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 4);
    if (HAL_GPIO_ReadPin(TRACK5_GPIO_Port, TRACK5_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 5);
    if (HAL_GPIO_ReadPin(TRACK6_GPIO_Port, TRACK6_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 6);
    if (HAL_GPIO_ReadPin(TRACK7_GPIO_Port, TRACK7_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 7);
    
    sensor_value = sensor_val; 
    return sensor_val;
}

// 辅助：计算黑线(0)的数量
int Count_Zeros(uint8_t val) {
    int count = 0;
    for(int i=0; i<8; i++) {
        if((val & (1<<i)) == 0) count++;
    }
    return count;
}

int16_t Get_Error(uint8_t sensor_state)
{
    // 1. 如果全黑(0x00)或绝大部分黑 -> 十字路口
    // 只要有 7个或8个 是黑的，直接判定为十字路口
    if (Count_Zeros(sensor_state) >= 7) {
        return ERR_CROSSROAD;
    }
    
    // 2. 左直角匹配 (0=黑, 1=白)
    // 0000 1111 (0x0F), 0000 0111 (0x07), 0000 0011 (0x03)
    if (sensor_state == 0x0F || sensor_state == 0x07 || sensor_state == 0x03)
    {
        return ERR_LEFT_TURN; 
    }

    // 3. 右直角匹配
    // 1111 0000 (0xF0), 1110 0000 (0xE0), 1100 0000 (0xC0)
    if (sensor_state == 0xF0 || sensor_state == 0xE0 || sensor_state == 0xC0)
    {
        return ERR_RIGHT_TURN; 
    }

    // 4. 普通PID计算
    if (sensor_state == 0xFF) return Last_Error; // 全白

    int32_t error_sum = 0;
    int count = 0;
    
    if (sensor_state & (1 << 7)) { error_sum -= 50; count++; }
    if (sensor_state & (1 << 6)) { error_sum -= 40; count++; }
    if (sensor_state & (1 << 5)) { error_sum -= 20; count++; }
    if (sensor_state & (1 << 4)) { error_sum -= 10; count++; }
    
    if (sensor_state & (1 << 3)) { error_sum += 10; count++; }
    if (sensor_state & (1 << 2)) { error_sum += 20; count++; }
    if (sensor_state & (1 << 1)) { error_sum += 40; count++; }
    if (sensor_state & (1 << 0)) { error_sum += 50; count++; }

    if (count == 0) return Last_Error;
    return (int16_t)(error_sum / count);
}

void Tracking_Handler(void)
{
    uint8_t current_sensor = Track_Read();
    int16_t error = Get_Error(current_sensor);

    float turn_speed = 60.0f; 

    // ==========================================================
    // 【关键修改：二次确认逻辑】
    // ==========================================================
    
    // 如果初次检测到是 十字路口 或者是 直角弯
    if (error == ERR_CROSSROAD || error == ERR_LEFT_TURN || error == ERR_RIGHT_TURN)
    {
        // 1. 先别急着转！让车子维持当前的动作(或稍微直行) 再往前蹭 30ms
        // 这个时间足够车子从“路口边缘”开进“路口中央”
        Target_Speed_L = TRACK_BASE_SPEED;
        Target_Speed_R = TRACK_BASE_SPEED;
        
        // 这是一个极短的盲跑，目的是更新传感器状态
        HAL_Delay(30); 
        
        // 2. 【再次读取】现在车子已经往前走了一点点了，看看还是不是直角？
        uint8_t next_sensor = Track_Read();
        
        // 3. 再次判断
        // 如果现在变成了全黑 (或者 >=6个黑)，说明刚才那个“直角”其实是十字路口的边缘！
        if (Count_Zeros(next_sensor) >= 6) 
        {
            // === 确认是十字路口 ===
            // 策略：大力直行冲过去
            Target_Speed_L = TRACK_BASE_SPEED + 10; //稍微加速冲
            Target_Speed_R = TRACK_BASE_SPEED + 10;
            
            // 冲过黑线的延时 (根据车速调整，150-200ms)
            HAL_Delay(200); 
            return; 
        }
        else 
        {
            // === 确认不是十字路口 ===
            // 说明真的是直角弯 (或者T字路口)，执行之前的判断
            
            if (error == ERR_LEFT_TURN) 
            {
                Target_Speed_L = -turn_speed; 
                Target_Speed_R = turn_speed;
                HAL_Delay(200);
                return;
            }
            else if (error == ERR_RIGHT_TURN)
            {
                Target_Speed_L = turn_speed;
                Target_Speed_R = -turn_speed;
                HAL_Delay(200);
                return;
            }
        }
    }

    // ==========================================================
    // 【普通 PID 区】
    // ==========================================================

    float speed_diff = (TRACK_KP * error) + (TRACK_KD * (error - Last_Error));
    Last_Error = error;

    float target_L = TRACK_BASE_SPEED - speed_diff;
    float target_R = TRACK_BASE_SPEED + speed_diff;

    if (target_L < 0) target_L = 0;
    if (target_L > 90) target_L = 90; 
    
    if (target_R < 0) target_R = 0;
    if (target_R > 90) target_R = 90;

    Target_Speed_L = target_L;
    Target_Speed_R = target_R;
}