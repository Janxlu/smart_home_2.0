#ifndef __BEEP_H
#define __BEEP_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_i2c.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"

//蜂鸣器控制引脚相关
#define BEEP_PIN_NAME WIFI_IOT_IO_NAME_GPIO_9
#define BEEP_PIN_FUNCTION WIFI_IOT_IO_FUNC_GPIO_9_PWM0_OUT
#define BEEP_GPIO_DIR      WIFI_IOT_GPIO_DIR_OUT
//使用PWM0
#define BEEP_PWM           WIFI_IOT_PWM_PORT_PWM0

// 修正PWM参数 - 使用合理的值
#define BEEP_PWM_FREQ  34052    // 频率分频器，约2kHz
#define BEEP_PWM_DUTY  (BEEP_PWM_FREQ / 2)  // 50%占空比

void Beep_Init(void);
void Beep_Start(void);
void Beep_Stop(void);

#endif