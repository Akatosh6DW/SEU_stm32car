#include "tracking.h"
#include "encoder.h" // 为了修改 Target_Speed
#include "gpio.h"

static int16_t Last_Error = 0;

uint8_t Track_Read(void)
{
    uint8_t sensor_val = 0;
    // 使用 main.h 的宏
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
    if (sensor_state == 0) return Last_Error;
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
    // 1. 读取传感器 & 计算位置误差
    uint8_t current_sensor = Track_Read();
    int16_t error = Get_Error(current_sensor);

    // 2. 循迹 PID 计算 (输出 = 左右轮的目标速度差)
    float speed_diff = (TRACK_KP * error) + (TRACK_KD * (error - Last_Error));
    Last_Error = error;

    // 3. 设定左右轮的目标速度 (不是 PWM!)
    // 基础速度 20，如果偏左(error负)，speed_diff负
    // Target_L = 20 + (-5) = 15 (减速)
    // Target_R = 20 - (-5) = 25 (加速) -> 向左修正
    float target_L = TRACK_BASE_SPEED - speed_diff;
    float target_R = TRACK_BASE_SPEED + speed_diff;

    // 4. 限幅 (防止倒车或超速)
    if (target_L < 0) target_L = 0;
    if (target_L > 90) target_L = 90; // 假设最大跑90cm/s
    
    if (target_R < 0) target_R = 0;
    if (target_R > 90) target_R = 90;

    // 5. 【关键】更新全局变量，让 encoder.c 去执行
    Target_Speed_L = target_L;
    Target_Speed_R = target_R;
    
}