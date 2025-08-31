#ifndef __LED_H
#define __LED_H

#include <stdio.h>

#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

//因为主控的IO口较少，目前只使用GREEN，
#define YELLOW_GPIO  WIFI_IOT_IO_NAME_GPIO_10
#define RED_GPIO     WIFI_IOT_IO_NAME_GPIO_10
#define GREEN_GPIO   WIFI_IOT_IO_NAME_GPIO_2

#define YELLOW_GPIO_FUNC  WIFI_IOT_IO_FUNC_GPIO_10_GPIO
#define RED_GPIO_FUNC     WIFI_IOT_IO_FUNC_GPIO_10_GPIO
#define GREEN_GPIO_FUNC   WIFI_IOT_IO_FUNC_GPIO_2_GPIO

#define LED_GPIO_DIR      WIFI_IOT_GPIO_DIR_OUT  

//灯种类枚举
typedef enum{
    RED=0,
    GREEN,
    YELLOW,
} LED_TYPE;

//灯开关状态枚举
typedef enum{
    OFF=0,
    ON,
}LED_CTRL;

void Led_Init(void);
void Led_Set(LED_TYPE led_type,LED_CTRL led_ctrl);
#endif /*__LED_H*/