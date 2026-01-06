#ifndef __TRACKING_H__
#define __TRACKING_H__

#include "main.h"

// --- 循迹参数 (串级控制版) ---
// 基础速度 (单位: cm/s) -> 建议设 20-30，不要太大
#define TRACK_BASE_SPEED   30.0f  

// 循迹 PID (输出的是速度差)
// 比如误差40，KP=0.5，则左右轮速度差 20cm/s (一个10，一个30)
#define TRACK_KP           7.5f   // 建议 0.3 - 1.0
#define TRACK_KD           0.5f   // 微分项，抑制震荡

void Tracking_Handler(void);

#endif