#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "main.h"

// 定义电机 ID
typedef enum {
    MOTOR_LF = 0, // 左前 (Left Front)
    MOTOR_LR,     // 左后 (Left Rear)
    MOTOR_RF,     // 右前 (Right Front)
    MOTOR_RR      // 右后 (Right Rear)
} Motor_ID_t;

// 定义最大 PWM 值 (对应 ARR)
#define MOTOR_MAX_PWM 1000
// --- 新增：全局方向变量声明 ---
// 1:前进, -1:后退, 0:停止
extern int8_t Motor_Dir_L;
extern int8_t Motor_Dir_R;
// 函数声明
void Motor_Init(void);
void Motor_Stop(void);
void Motor_SetSpeed(Motor_ID_t motor_id, int16_t speed);

#endif /* __MOTOR_H__ */