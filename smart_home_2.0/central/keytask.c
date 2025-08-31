#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "wifiiot_adc.h"

#include "oled_ssd1306.h"
#include "tcp_connect.h"

//一共使用到三个按键，其中两个按键是模拟按键，接到一个ADC通道，通过ADC值判断两个按键状态。另外一个是普通按键
//两个模拟按键s使用ADC2通道
#define ANALOG_KEY_CHAN_NAME WIFI_IOT_ADC_CHANNEL_2
//普通按键IO口
#define BUTTON_GPIO          WIFI_IOT_IO_NAME_GPIO_6
#define BUTTON_GPIO_FUNC     WIFI_IOT_IO_FUNC_GPIO_6_GPIO
#define BUTTON_GPIO_DIR      WIFI_IOT_GPIO_DIR_IN

//显示界面枚举类型
typedef enum
{
    TIMESCREEN=0,       //时间显示界面，开机后的界面
    NOWSCREEN,          //现在天气情况的显示界面
    TOSCREEN,           //明天天气情况的显示界面
    ATOSCREEN,          //后天天气情况的显示界面
    BCTIMESCREEN,       //布撤防时间调整的显示界面
    BCADJUSTSCEEN,      //设置手自动模式、布撤防状态的界面
    NOSCREEN,           //无界面
} SCREEN_STATUS;

extern SCREEN_STATUS Now_Screen ;
extern SCREEN_STATUS Last_Screen ;

//时间和天气数据获取情况枚举类型
typedef enum
{
    GET_NORMAL =0,      //没在获取数据
    GET_PROPRESS,       //获取进行中
    GET_SUC,            //获取成功
    GET_FAIL,           //获取失败
} GET_STATUS;

//新的获取状态
GET_STATUS Get_Status = GET_NORMAL;
//之前的获取状态
GET_STATUS Last_Get_Status = GET_NORMAL;

extern bool getNtpTime(void);
extern bool getWeather(void);

extern uint8_t bcflag ;
extern uint8_t zdflag ;

extern uint8_t starttime ;
extern uint8_t stoptime ;
extern uint8_t ledflag  ;

//Led灯控制函数
static void LedContrcl(void)
{
    if (ledflag)
    {
        ledflag = 0 ;
    }
    else
    {
        ledflag = 1;
    }


}

//转化为电压值
static float ConvertToVoltage(unsigned short data)
{
    return (float)data * 1.8 * 4 / 4096; /* adc code equals: voltage/4/1.8*4096  */
}

//布防时间调整
static void BfTimeAdjust(void)
{
    starttime++;
    if(starttime>23)
        starttime=0;
}

//撤防时间调整
static void CfTimeAdjust(void)
{
    stoptime++;
    if(stoptime>23)
        stoptime=0;
}

//手自动模式调整
static void ZdAdjust(void)
{
    if(zdflag)
    {
        zdflag = 0;
    }
    else
    {
        zdflag = 1;
    }
}

//布撤防状态调整
static void BcAdjust(void)
{
    if(!zdflag)     //手动模式下，才能进行布撤防操作
    {
        if(bcflag)
            bcflag = 0;
        else
        {
            bcflag = 1;
        }
    }
}

//按键任务
static void KeyTask(void *arg)
{
    (void)arg;
    unsigned short data = 0;

    //按键IO初始化
    GpioInit();
    IoSetFunc(BUTTON_GPIO, BUTTON_GPIO_FUNC);
    GpioSetDir(BUTTON_GPIO, BUTTON_GPIO_DIR);
    IoSetPull(BUTTON_GPIO,WIFI_IOT_IO_PULL_UP);     //设置为上拉

    //按键按下标志，按下置1,松开置0
    static bool keyflag = false;
    static bool buttonflag =false;
    while(1)
    {
        //获取模拟按键状态
        AdcRead(ANALOG_KEY_CHAN_NAME, &data, WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0);
        float voltage = ConvertToVoltage(data);

        //显示板的左边按键按下
        if((voltage>0.45 && voltage<0.65)&&(!keyflag))  //一直按下，不会重复操作，只有松开再按下，才能再次进入
        {
            keyflag = true;     
            //在不同的显示界面上，按键按下的操作都不一样
            switch (Now_Screen)
            {
            case TIMESCREEN:                        //在时间显示界面时，获取实时时间
                if(connect_status == CONNECTED)
                    TcpDisconnect();                //需要先断开之前的网络连接
                Get_Status = GET_PROPRESS;
                if(getNtpTime())
                {
                    Get_Status = GET_SUC;
                }
                else
                {
                    Get_Status = GET_FAIL;
                }
                TcpConnect();                       //恢复之前的网络连接
                break;
            case NOWSCREEN:                         //在现在天气显示界面、明天天气显示界面、后天天气显示界面，都是获取天气数据
                if(connect_status == CONNECTED) 
                    TcpDisconnect();
                Get_Status = GET_PROPRESS;
                if(getWeather())
                    Get_Status = GET_SUC;
                else
                {
                    Get_Status = GET_FAIL;
                }
                TcpConnect();
                break;
            case TOSCREEN:                          //在现在天气显示界面、明天天气显示界面、后天天气显示界面，都是获取天气数据
                if(connect_status == CONNECTED)
                    TcpDisconnect();
                Get_Status = GET_PROPRESS;
                if(getWeather())
                    Get_Status = GET_SUC;
                else
                {
                    Get_Status = GET_FAIL;
                }
                TcpConnect();
                break;
            case ATOSCREEN:                         //在现在天气显示界面、明天天气显示界面、后天天气显示界面，都是获取天气数据
                if(connect_status == CONNECTED)
                    TcpDisconnect();
                Get_Status = GET_PROPRESS;
                if(getWeather())
                    Get_Status = GET_SUC;
                else
                {
                    Get_Status = GET_FAIL;
                }
                TcpConnect();
                break;
            case BCTIMESCREEN:                      //在时间调整界面，调整布防时间
                BfTimeAdjust();
                break;
            case BCADJUSTSCEEN:                     //在模式设置界面，切换手自动模式
                ZdAdjust();
                break;
            default:
                break;
            }

        }
        else if((voltage>0.9 && voltage<1)&&(!keyflag))    //显示板右边按键按下，切换界面
        {
            keyflag = true;
            Now_Screen ++;
            if(Now_Screen >= NOSCREEN)
            {
                Now_Screen = TIMESCREEN;
            }
        }
        //按键松开
        if((!(voltage>0.45 && voltage<0.65)) && (!(voltage>0.9 && voltage<1)))
        {
            keyflag = false;
        }
        
        //红黄绿灯板按键状态
        WifiIotGpioValue buttonValue;
        GpioGetInputVal(BUTTON_GPIO,&buttonValue);
        //按键按下
        if((buttonValue == WIFI_IOT_GPIO_VALUE0)&&(!buttonflag))
        {
            switch (Now_Screen)
            {
            case TIMESCREEN:        //在时间显示界面，打开或关闭LED灯
                LedContrcl();
                break;
            case NOWSCREEN:
            case TOSCREEN:
            case ATOSCREEN:
                break;
            case BCTIMESCREEN:      //在时间设置界面，调整撤防时间
                CfTimeAdjust();
                break;
            case BCADJUSTSCEEN:     //在模式设置界面，切换布撤防状态
                BcAdjust();
                break;
            default:
                break;
            }
            buttonflag = true;
        }
        //按键松开
        if(!(buttonValue == WIFI_IOT_GPIO_VALUE0))
        {
            buttonflag = false;
        }
        usleep(100000);
    }
}

static void KeyTaskHandle(void)
{
    osThreadAttr_t attr;
    attr.name = "KeyTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal+2;

    if (osThreadNew(KeyTask, NULL, &attr) == NULL)
    {
        printf("[KeyTaskHandle] Falied to create KeyTask!\n");
    }
}
APP_FEATURE_INIT(KeyTaskHandle);