#ifndef __ENCODER_H__
#define __ENCODER_H__

#include "main.h"

// --- 参数配置 ---
#define WHEEL_CIRCUMFERENCE  20.4f  // 轮子周长 (cm)
#define PULSES_PER_REV       1170   // 一圈脉冲数
// 【重要修改】采样周期改为 50ms (为了 PID 反应更灵敏)
#define SPEED_CALC_PERIOD    50     

// 全局变量
extern volatile int32_t Encoder_Count_L;
extern volatile int32_t Encoder_Count_R;
extern float Car_Speed_L;
extern float Car_Speed_R;

// --- 新增：全局目标速度 (由 Tracking 设定) ---
extern float Target_Speed_L;
extern float Target_Speed_R;

// 电机方向 (motor.c 定义)
extern int8_t Motor_Dir_L; 
extern int8_t Motor_Dir_R;

void Encoder_Init(void);

#endif