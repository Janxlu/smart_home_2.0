#include "beep.h"

//蜂鸣器IO口初始化
void Beep_Init(void){
    GpioInit();
    IoSetFunc(BEEP_PIN_NAME, BEEP_PIN_FUNCTION);
    GpioSetDir(BEEP_PIN_NAME, BEEP_GPIO_DIR);
    PwmInit(BEEP_PWM);
}

//蜂鸣器开始发声
void Beep_Start(void){
    PwmStart(BEEP_PWM, BEEP_PWM_DUTY, BEEP_PWM_FREQ);
}

//蜂鸣器停止发声
void Beep_Stop(void){
    PwmStop(BEEP_PWM);
}
