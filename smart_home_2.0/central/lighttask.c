
#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "wifiiot_adc.h"
#include "wifiiot_errno.h"

//光敏电阻接到的ADC通道
#define LIGHT_SENSOR_CHAN_NAME WIFI_IOT_ADC_CHANNEL_0

//三色灯的IO口
#define RED_LED_PIN_NAME WIFI_IOT_IO_NAME_GPIO_1
#define RED_LED_PIN_FUNCTION WIFI_IOT_IO_FUNC_GPIO_1_GPIO

#define GREEN_LED_PIN_NAME WIFI_IOT_IO_NAME_GPIO_0
#define GREEN_LED_PIN_FUNCTION WIFI_IOT_IO_FUNC_GPIO_0_GPIO

#define BLUE_LED_PIN_NAME WIFI_IOT_IO_NAME_GPIO_8
#define BLUE_LED_PIN_FUNCTION WIFI_IOT_IO_FUNC_GPIO_8_GPIO

//获取到的最大值
#define MAX_ADCVALUE 1900
#define PWM_FREQ_DIVITION 60000

//灯开关标志位，0为灯关，1为灯开
uint8_t ledflag = 0;
static void LightTask(void *arg)
{
    (void)arg;
    //初始化IO
    GpioInit();
    IoSetFunc(RED_LED_PIN_NAME, WIFI_IOT_IO_FUNC_GPIO_1_PWM4_OUT);
    IoSetFunc(GREEN_LED_PIN_NAME, WIFI_IOT_IO_FUNC_GPIO_0_PWM3_OUT);
    IoSetFunc(BLUE_LED_PIN_NAME, WIFI_IOT_IO_FUNC_GPIO_8_PWM1_OUT);
    GpioSetDir(RED_LED_PIN_NAME, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetDir(GREEN_LED_PIN_NAME, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetDir(BLUE_LED_PIN_NAME, WIFI_IOT_GPIO_DIR_OUT);
    //初始化PWM
    PwmInit(WIFI_IOT_PWM_PORT_PWM4); // R
    PwmInit(WIFI_IOT_PWM_PORT_PWM1); // B
    PwmInit(WIFI_IOT_PWM_PORT_PWM3); // G

    while (1) {
            unsigned short data = 0;
            unsigned short duty = 0;
            //获取光敏电阻的值
            if (AdcRead(LIGHT_SENSOR_CHAN_NAME, &data, WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0)
                == WIFI_IOT_SUCCESS) {
                //根据值调整PWM波
                duty = PWM_FREQ_DIVITION * (unsigned int)data/ MAX_ADCVALUE;
            }
            //灯为开启状态，开始PWM
            if(ledflag)
            {
            PwmStart(WIFI_IOT_PWM_PORT_PWM4, duty, PWM_FREQ_DIVITION);
            PwmStart(WIFI_IOT_PWM_PORT_PWM1, duty, PWM_FREQ_DIVITION);
            PwmStart(WIFI_IOT_PWM_PORT_PWM3, duty, PWM_FREQ_DIVITION);
            }
            else
            {
            //灯为关闭状态，停止PWM
            PwmStart(WIFI_IOT_PWM_PORT_PWM4, 0, PWM_FREQ_DIVITION);
            PwmStart(WIFI_IOT_PWM_PORT_PWM1, 0, PWM_FREQ_DIVITION);
            PwmStart(WIFI_IOT_PWM_PORT_PWM3, 0, PWM_FREQ_DIVITION);
            PwmStop(WIFI_IOT_PWM_PORT_PWM4);
            PwmStop(WIFI_IOT_PWM_PORT_PWM1);
            PwmStop(WIFI_IOT_PWM_PORT_PWM3);
            }
            usleep(10000);
    }
}

static void LightDemo(void)
{
    osThreadAttr_t attr;

    
    attr.name = "LightTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    if (osThreadNew(LightTask, NULL, &attr) == NULL) {
        printf("[LightDemo] Falied to create LightTask!\n");
    }
}

APP_FEATURE_INIT(LightDemo);
