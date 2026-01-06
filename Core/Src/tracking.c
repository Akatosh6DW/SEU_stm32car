#include "tracking.h"
#include "encoder.h" 
#include "gpio.h"
#include "stm32f4xx_hal.h"
#include <math.h>
uint8_t sensor_value = 0;
static int16_t Last_Error = 0;

// 定义特殊返回值
#define ERR_LEFT_TURN    -1000
#define ERR_RIGHT_TURN    1000

uint8_t Track_Read(void)
{
    // 【完全保留】你的原版读取函数
    uint8_t sensor_val = 0;
    if (HAL_GPIO_ReadPin(TRACK0_GPIO_Port, TRACK0_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 0);
    if (HAL_GPIO_ReadPin(TRACK1_GPIO_Port, TRACK1_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 1);
    if (HAL_GPIO_ReadPin(TRACK2_GPIO_Port, TRACK2_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 2);
    if (HAL_GPIO_ReadPin(TRACK3_GPIO_Port, TRACK3_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 3);
    if (HAL_GPIO_ReadPin(TRACK4_GPIO_Port, TRACK4_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 4);
    if (HAL_GPIO_ReadPin(TRACK5_GPIO_Port, TRACK5_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 5);
    if (HAL_GPIO_ReadPin(TRACK6_GPIO_Port, TRACK6_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 6);
    if (HAL_GPIO_ReadPin(TRACK7_GPIO_Port, TRACK7_Pin) == GPIO_PIN_SET) sensor_val |= (1 << 7);
    sensor_value = sensor_val; // 更新全局变量
    return sensor_val;
}

int16_t Get_Error(uint8_t sensor_state)
{
    // ==========================================================
    // 【绝对匹配区】按照你要求的二进制值 (0=黑, 1=白)
    // ==========================================================
    if (sensor_state == 0) return Last_Error;
    // --- 左直角判据 ---
    // 0000 1111 (0x0F) -> 左边4个黑
    // 0000 0111 (0x07) -> 左边3个黑, 右边1个白 (更灵敏)
    // 0000 0011 (0x03) -> 左边2个黑 (极灵敏)
    if (sensor_state == 0x0F || sensor_state == 0x07 || sensor_state == 0x03)
    {
        return ERR_LEFT_TURN; 
    }

    // --- 右直角判据 ---
    // 1111 0000 (0xF0) -> 右边4个黑
    // 1110 0000 (0xE0) -> 右边3个黑
    // 1100 0000 (0xC0) -> 右边2个黑
    if (sensor_state == 0xF0 || sensor_state == 0xE0 || sensor_state == 0xC0)
    {
        return ERR_RIGHT_TURN; 
    }

    // ==========================================================
    // 【保留：你的原版 PID 权重计算区】
    // (以下代码未改动)
    // ==========================================================

    
    int32_t error_sum = 0;
    int count = 0;
    
    // 左侧 (负分)
    // if (sensor_state & (1 << 7)) { error_sum -= 40; count++; }
    if (sensor_state & (1 << 7)) { error_sum -= 50; count++; }
    if (sensor_state & (1 << 6)) { error_sum -= 40; count++; }
    if (sensor_state & (1 << 5)) { error_sum -= 20; count++; }
    if (sensor_state & (1 << 4)) { error_sum -= 10; count++; }
    // 右侧 (正分)
    if (sensor_state & (1 << 3)) { error_sum += 10; count++; }
    if (sensor_state & (1 << 2)) { error_sum += 20; count++; }
    if (sensor_state & (1 << 1)) { error_sum += 40; count++; }
    if (sensor_state & (1 << 0)) { error_sum += 50; count++; }

    if (count == 0) return Last_Error;
    return (int16_t)(error_sum / count);
}

void Tracking_Handler(void)
{
    // 1. 读取 & 计算
    uint8_t current_sensor = Track_Read();
    int16_t error = Get_Error(current_sensor);

    // ==========================================================
    // 【拦截执行区】
    // ==========================================================
    
    float turn_speed = 60.0f; // 坦克掉头速度

    if (error == ERR_LEFT_TURN) 
    {
        // === 左直角 ===
        // 左轮退，右轮进
        Target_Speed_L = -turn_speed; 
        Target_Speed_R = turn_speed;
        HAL_Delay(200);
        return; // 直接跳出
    }
    else if (error == ERR_RIGHT_TURN)
    {
        // === 右直角 ===
        // 左轮进，右轮退
        Target_Speed_L = turn_speed;
        Target_Speed_R = -turn_speed;
        HAL_Delay(350);
        return; // 直接跳出
    }

    // ==========================================================
    // 【保留：你的原版 PID 执行区】
    // ==========================================================

    // 2. 循迹 PID 计算
    float speed_diff = (TRACK_KP * error) + (TRACK_KD * (error - Last_Error));
    Last_Error = error;

    // 3. 设定左右轮的目标速度
    float target_L = TRACK_BASE_SPEED - speed_diff;
    float target_R = TRACK_BASE_SPEED + speed_diff;

    // 4. 限幅 (禁止负数)
    if (target_L < 0) target_L = 0;
    if (target_L > 90) target_L = 90; 
    
    if (target_R < 0) target_R = 0;
    if (target_R > 90) target_R = 90;

    // 5. 更新全局变量
    Target_Speed_L = target_L;
    Target_Speed_R = target_R;
}