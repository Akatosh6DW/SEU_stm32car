#ifndef __PID_H__
#define __PID_H__

#include "main.h"

// --- 速度环 PID 参数 (需要根据实测微调) ---
// KP: 响应力度 (建议 5.0 - 20.0)
// KI: 消除静态误差 (建议 0.5 - 2.0)
// KD: 速度环通常为 0
#define PID_KP  35.0f   
#define PID_KI  0.0f    
#define PID_KD  0.5f    

// 限制
#define PID_MAX_OUT  950.0f   // PWM 最大值
#define PID_MAX_I    400.0f   // 积分限幅

typedef struct {
    float Target;      // 目标值
    float Measured;    // 实际值
    float Error;       // 误差
    float LastError;   // 上次误差
    float Integral;    // 积分
    float Output;      // 输出
} PID_TypeDef;

// 外部声明
extern PID_TypeDef PID_L;
extern PID_TypeDef PID_R;

void PID_Init(PID_TypeDef *pid);
float PID_Compute(PID_TypeDef *pid, float target, float measured);

#endif