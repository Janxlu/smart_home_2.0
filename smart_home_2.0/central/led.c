#include "led.h"

static WifiIotIoName type_arr[]={RED_GPIO,GREEN_GPIO,YELLOW_GPIO};
static LED_CTRL ctrl_arr[]={OFF,ON};

//灯IO口初始化
void Led_Init(void){

    GpioInit();
    IoSetFunc(RED_GPIO,  RED_GPIO_FUNC);
    GpioSetDir(RED_GPIO, LED_GPIO_DIR);
   
    IoSetFunc(GREEN_GPIO,  GREEN_GPIO_FUNC);
    GpioSetDir(GREEN_GPIO, LED_GPIO_DIR);
    
    IoSetFunc(YELLOW_GPIO,  YELLOW_GPIO_FUNC);
    GpioSetDir(YELLOW_GPIO, LED_GPIO_DIR);

    GpioSetOutputVal(type_arr[RED], ctrl_arr[OFF]);
    GpioSetOutputVal(type_arr[GREEN], ctrl_arr[OFF]);
    GpioSetOutputVal(type_arr[YELLOW], ctrl_arr[OFF]);
}

//设置红绿黄灯的亮灭
void Led_Set(LED_TYPE led_type,LED_CTRL led_ctrl){
     GpioSetOutputVal(type_arr[led_type], ctrl_arr[led_ctrl]);
}