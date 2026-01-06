#include "pid.h"

PID_TypeDef PID_L;
PID_TypeDef PID_R;

void PID_Init(PID_TypeDef *pid)
{
    pid->Target = 0;
    pid->Measured = 0;
    pid->Error = 0;
    pid->LastError = 0;
    pid->Integral = 0;
    pid->Output = 0;
}

float PID_Compute(PID_TypeDef *pid, float target, float measured)
{
    pid->Target = target;
    pid->Measured = measured;
    
    pid->Error = pid->Target - pid->Measured;

    float P = PID_KP * pid->Error;

    pid->Integral += pid->Error;
    // 积分抗饱和
    if (pid->Integral > PID_MAX_I) pid->Integral = PID_MAX_I;
    else if (pid->Integral < -PID_MAX_I) pid->Integral = -PID_MAX_I;
    float I = PID_KI * pid->Integral;

    float D = PID_KD * (pid->Error - pid->LastError);

    pid->Output = P + I + D;
    pid->LastError = pid->Error;

    // 输出限幅
    if (pid->Output > PID_MAX_OUT) pid->Output = PID_MAX_OUT;
    else if (pid->Output < -PID_MAX_OUT) pid->Output = -PID_MAX_OUT;

    return pid->Output;
}