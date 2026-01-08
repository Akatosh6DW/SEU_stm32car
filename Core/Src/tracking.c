#include "tracking.h"
#include "encoder.h" 
#include "gpio.h"
#include "stm32f4xx_hal.h"
#include <math.h>

// Global variables
uint8_t sensor_value = 0;
static int16_t Last_Error = 0;

// Special Return Values
#define ERR_LEFT_TURN    -1000
#define ERR_RIGHT_TURN    1000
#define ERR_CROSSROAD     5000 

uint8_t Track_Read(void)
{
    // [Keep Original Reading Logic]
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

// Helper: Count zeros (black lines)
int Count_Zeros(uint8_t val) {
    int count = 0;
    for(int i=0; i<8; i++) {
        if((val & (1<<i)) == 0) count++;
    }
    return count;
}

int16_t Get_Error(uint8_t sensor_state)
{
    // 1. Crossroad Check
    if (Count_Zeros(sensor_state) >= 7) {
        return ERR_CROSSROAD;
    }
    
    // 2. Left Turn Pattern
    // 0000 1111 (0x0F), 0000 0111 (0x07), 0000 0011 (0x03)
    if (sensor_state == 0x0F || sensor_state == 0x07 || sensor_state == 0x03)
    {
        return ERR_LEFT_TURN; 
    }

    // 3. Right Turn Pattern
    // 1111 0000 (0xF0), 1110 0000 (0xE0), 1100 0000 (0xC0)
    if (sensor_state == 0xF0 || sensor_state == 0xE0 || sensor_state == 0xC0)
    {
        return ERR_RIGHT_TURN; 
    }

    // 4. Normal PID Error Calculation
    if (sensor_state == 0xFF) return Last_Error; // All white

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
    // [Double Check Logic for Crossroads/Turns]
    // ==========================================================
    if (error == ERR_CROSSROAD || error == ERR_LEFT_TURN || error == ERR_RIGHT_TURN)
    {
        Target_Speed_L = TRACK_BASE_SPEED;
        Target_Speed_R = TRACK_BASE_SPEED;
        
        // Blind move forward slightly to check again
        HAL_Delay(30); 
        
        uint8_t next_sensor = Track_Read();
        
        if (Count_Zeros(next_sensor) >= 6) 
        {
            // === Confirmed Crossroad ===
            Target_Speed_L = TRACK_BASE_SPEED + 10; 
            Target_Speed_R = TRACK_BASE_SPEED + 10;
            HAL_Delay(200); 
            return; 
        }
        else 
        {
            // === Confirmed Sharp Turn ===
            // This is the "Hard Logic" turn
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
    // [Normal PID Section] - NOW WITH REVERSE ENABLED
    // ==========================================================

    float speed_diff = (TRACK_KP * error) + (TRACK_KD * (error - Last_Error));
    Last_Error = error;

    float target_L = TRACK_BASE_SPEED - speed_diff;
    float target_R = TRACK_BASE_SPEED + speed_diff;

    // [CRITICAL CHANGE HERE]
    // We allow negative values (reverse) to help fight oscillation on tight curves
    // But we limit the reverse speed so it doesn't go crazy.
    
    float max_fwd = 90.0f; // Max forward speed
    float max_rev = -40.0f; // Max reverse speed (don't make this too high!)

    // Left Wheel Limit
    if (target_L > max_fwd) target_L = max_fwd;
    if (target_L < max_rev) target_L = max_rev; // Allow reverse down to -40
    
    // Right Wheel Limit
    if (target_R > max_fwd) target_R = max_fwd;
    if (target_R < max_rev) target_R = max_rev; // Allow reverse down to -40

    Target_Speed_L = target_L;
    Target_Speed_R = target_R;
}