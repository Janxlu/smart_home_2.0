#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "wifiiot_adc.h"

#include "oled_ssd1306.h"
#include "timeconv.h"
#include "cjsonparse.h"


//在 oledtask.c 中添加一个全局标志，表示是否处于火灾报警状态
extern int fire_alarm;  // 来自 envtask.c


// 时间戳值
extern uint32_t timedata;

static uint8_t TimeStr[30];
static uint8_t DispStr[10];

// 温湿度
extern float humidity;
extern float temperature;

// 显示界面枚举类型
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

// 现在需要显示的界面
SCREEN_STATUS Now_Screen = TIMESCREEN;
// 之前显示的界面
SCREEN_STATUS Last_Screen = TIMESCREEN;

// 时间和天气数据获取情况枚举类型
typedef enum
{
    GET_NORMAL =0,      //没在获取数据
    GET_PROPRESS,       //获取进行中
    GET_SUC,            //获取成功
    GET_FAIL,           //获取失败
} GET_STATUS;

extern GET_STATUS Get_Status ;
extern GET_STATUS Last_Get_Status ;

static uint8_t i=0;

// 布撤防时间，小时
uint8_t starttime = 8;
uint8_t stoptime = 18;
// 布撤防状态标志位，0：撤防，1：布防
uint8_t bcflag = 0;
// 手自动模式标志位 0：手动，1：自动
uint8_t zdflag = 0;
// 烟雾报警标志位， 0：正常，1：报警
uint8_t ywflag = 0;
// 防盗报警标志位   0：正常，1：报警
uint8_t rtflag = 0;
// 现在时间的小时
uint8_t nowhour = 0;

// 按键处理函数声明
void KeyHandler(void);

// 在 oledtask.c 中添加以下函数，火灾跳转
void ShowFireAlarmScreen(void)
{
     OledFillScreen(0x00); // 清屏
    usleep(1000); // 短暂延迟确保清屏完成
    OLED_ShowCHinese(32, 2, 20); // “火”
    OLED_ShowCHinese(48, 2, 21); // “灾”
    OLED_ShowCHinese(64, 2, 22); // “报”
    OLED_ShowCHinese(80, 2, 23); // “警”
    OledShowString(10, 5, "FIRE DETECTED!", FONT8x16);
     sleep(1);

}

// 按键处理函数 - 切换显示界面
void KeyHandler(void)
{
    // 直接根据当前界面切换到下一个界面，跳过NOSCREEN
    switch (Now_Screen)
    {
        case TIMESCREEN:
            Now_Screen = NOWSCREEN;
            break;
        case NOWSCREEN:
            Now_Screen = TOSCREEN;
            break;
        case TOSCREEN:
            Now_Screen = ATOSCREEN;
            break;
        case ATOSCREEN:
            Now_Screen = BCTIMESCREEN;
            break;
        case BCTIMESCREEN:
            Now_Screen = BCADJUSTSCEEN;
            break;
        case BCADJUSTSCEEN:
            Now_Screen = TIMESCREEN;
            break;
        default:
            Now_Screen = TIMESCREEN; // 默认回到时间界面
            break;
    }
    printf("切换到界面: %d\n", Now_Screen); // 调试信息
}

// 时间显示界面，手自动模式、布撤防状态显示
static void DefendDisplay(void)
{
    if(bcflag)
    {
        OLED_ShowCHinese(96,5,6);//布
        OLED_ShowCHinese(112,5,8);//防
    }
    else
    {
        OLED_ShowCHinese(96,5,7);//撤
        OLED_ShowCHinese(112,5,8);//防
    }
    if(zdflag)
    {
        OLED_ShowCHinese(0,5,4);//自
        OLED_ShowCHinese(16,5,5);//动
    }
    else
    {
        OLED_ShowCHinese(0,5,3);//手
        OLED_ShowCHinese(16,5,5);//动
    }
}

/*自动布撤防控制*/
void Zdbcf(void)
{
    if(zdflag)   //已经开启自动模式
    {
        if(starttime<stoptime)  //布防时间小于撤防时间
        {
            if(nowhour<stoptime && nowhour >= starttime)
            {
                bcflag=1;       //布防
            }
            else
                bcflag=0;       //撤防
        }
        if(starttime>stoptime) //布防时间大于撤防时间
        {
            if(nowhour>=starttime || nowhour<stoptime )
            {
                bcflag=1;       //布防
            }
            else
                bcflag=0;       //撤防
        }
        if(stoptime==starttime) //布撤防时间相同，撤防
            bcflag=0;           //撤防
    }
}

// 时间显示界面显示内容
static void TimeScreenDisp(void)
{
    rtc_time_t mData;
    //显示温湿度
    OLED_ShowCHinese(0,3,0);//温
    OLED_ShowCHinese(16,3,2);//度
    OledShowChar(32,3,':',2);
    sprintf((char *)TimeStr,"%02d", (uint32_t)temperature);
    OledShowString(40,3,(char *)TimeStr,2);
    OLED_ShowCHinese(64,3,1);//湿
    OLED_ShowCHinese(80,3,2);//度
    OledShowChar(96,3,':',2);
    sprintf((char *)TimeStr,"%02d", (uint32_t)humidity);
    OledShowString(104,3,(char *)TimeStr,2);
    
    //显示时间
    covUnixTimeStp2Beijing(timedata, &mData);
    sprintf((char *)TimeStr,"%04d-%02d-%02d", mData.ui8Year, mData.ui8Month, mData.ui8DayOfMonth);
    OledShowString(24,0,(char *)TimeStr,1);
    sprintf((char *)TimeStr,"%02d:%02d:%02d", mData.ui8Hour,mData.ui8Minute,mData.ui8Second);
    OledShowString(32,1,(char *)TimeStr,1);
    
    //显示布撤防时间
    sprintf((char *)TimeStr,"%02d:00-%02d:00",starttime,stoptime);
    OledShowString(20,2,(char *)TimeStr,1);
    nowhour = mData.ui8Hour;

    //根据获取状态，显示获取时间状态提示
    if(Get_Status != Last_Get_Status)
    {
        Last_Get_Status = Get_Status;
        switch (Get_Status)
        {
        case GET_NORMAL:
            break;
        case GET_PROPRESS:
            OledShowString(16,7,"Sync time...",1); 
            break;
        case GET_SUC:
            OledClearString(16,7,"Sync time...",1);
            OledShowString(16,7,"Sync Suc...",1);
            break;
        case GET_FAIL:
            OledClearString(16,7,"Sync time...",1);
            OledShowString(16,7,"Sync fail...",1);
            break;
        default:
            break;
        }
    }
}

//显示天气图标
static void DispWeather(uint8_t x, uint8_t y,uint8_t code)
{
    switch(code)
    {
    case 0:
    case 1:
    case 2:
    case 3:
        OLED_ShowWeather(x,y,QINGLOGO);
        break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        OLED_ShowWeather(x,y,DUOYULOGO);
        break;
    case 9:
    case 10:
        OLED_ShowWeather(x,y,YINGLOGO);
        break;
    case 11:
    case 12:
        OLED_ShowWeather(x,y,LEIYULOGO);
        break;
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
        OLED_ShowWeather(x,y,YULOGO);
        break;
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
        OLED_ShowWeather(x,y,XUELOGO);
        break;
    case 30:
    default:
        OLED_ShowWeather(x,y,QINGLOGO);
        break;
    }
}

extern weather weatherValue;

//天气显示界面内容，包括今天、明天、后天
static void WeatherDisp(SCREEN_STATUS Screen)
{
    switch (Screen)
    {
    case NOWSCREEN:                                 //今天
        DispWeather(5, 1,weatherValue.nowcode);     //显示图标
        OLED_ShowCHinese(56,0,9);//今
        OLED_ShowCHinese(72,0,12);//天
        sprintf((char *)DispStr,"%02d", (uint32_t)weatherValue.nowtemp);    //温度
        OledShowString(60,5,(char *)DispStr,2);
        OLED_ShowCHinese(76,5,13);//℃
        sprintf((char *)DispStr,"%02d/%02d",        //最高最低温度
                (uint32_t)weatherValue.low[Screen-1],(uint32_t)weatherValue.high[Screen-1]);
        OledShowString(60,3,(char *)DispStr,2);
        sprintf((char *)DispStr,"%d%%", (uint32_t)weatherValue.humi[Screen-1]);
        OledShowString(100,5,(char *)DispStr,2);    //湿度
        break;
    case TOSCREEN:                                  //明天
        DispWeather(5, 1,weatherValue.code[Screen-1]);
        OLED_ShowCHinese(56,0,10);//明
        OLED_ShowCHinese(72,0,12);//天
        sprintf((char *)DispStr,"%02d/%02d",        //最高最低温度
                (uint32_t)weatherValue.low[Screen-1],(uint32_t)weatherValue.high[Screen-1]);
        OledShowString(60,3,(char *)DispStr,2);
        sprintf((char *)DispStr,"%d%%", (uint32_t)weatherValue.humi[Screen-1]);     //湿度
        OledShowString(70,5,(char *)DispStr,2);
        break;
    case ATOSCREEN:                                         //后天
        DispWeather(5, 1,weatherValue.code[Screen-1]);
        OLED_ShowCHinese(56,0,11);//后
        OLED_ShowCHinese(72,0,12);//天
        sprintf((char *)DispStr,"%02d/%02d",                //最高最低温度
                (uint32_t)weatherValue.low[Screen-1],(uint32_t)weatherValue.high[Screen-1]);
        OledShowString(60,3,(char *)DispStr,2);
        sprintf((char *)DispStr,"%d%%", (uint32_t)weatherValue.humi[Screen-1]);
        OledShowString(70,5,(char *)DispStr,2);             //湿度
        break;
    default:
        break;
    }

    //根据获取状态，显示获取天气数据状态提示
    if(Get_Status != Last_Get_Status)
    {
        Last_Get_Status = Get_Status;
        switch (Get_Status)
        {
        case GET_NORMAL:
            break;
        case GET_PROPRESS:
            OledShowString(0,7,"Get Weather...",1);
            break;
        case GET_SUC:
            OledClearString(0,7,"Get Weather...",1);
            OledShowString(0,7,"Get Suc...",1);
            i=0;
            break;
        case GET_FAIL:
            OledClearString(0,7,"Get Weather...",1);
            OledShowString(0,7,"Get fail...",1);
            i=0;
            break;
        default:
            break;
        }
    }
}

static void OledTask(void *arg)
{
    (void)arg;
    // Oled初始化
    GpioInit();
    OledInit();
    OledFillScreen(0x00);
    
    // 按键GPIO初始化 - 使用正确的Hi3861 GPIO函数
    // 设置GPIO5为GPIO功能
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_IO_FUNC_GPIO_5_GPIO);
    // 设置GPIO5为输入模式
    GpioSetDir(WIFI_IOT_GPIO_IDX_5, WIFI_IOT_GPIO_DIR_IN);
    // 设置GPIO5上拉电阻
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_IO_PULL_UP);

    while (1)
    {
        // 如果正在火灾报警，则跳过所有其他界面逻辑
        if (fire_alarm)
        {
            usleep(100000);
            continue; // 跳过本次循环，不刷新其他界面
        }
        
        // 检测按键 - 使用正确的GpioGetInputVal函数
        WifiIotGpioValue keyValue;
        unsigned int ret = GpioGetInputVal(WIFI_IOT_GPIO_IDX_5, &keyValue);
        
        if (ret == 0 && keyValue == WIFI_IOT_GPIO_VALUE0) { // WIFI_IOT_SUCCESS 应该是0
            usleep(10000); // 简单消抖
            
            ret = GpioGetInputVal(WIFI_IOT_GPIO_IDX_5, &keyValue);
            if (ret == 0 && keyValue == WIFI_IOT_GPIO_VALUE0) {
                KeyHandler(); // 调用按键处理函数
                
                // 等待按键释放
                do {
                    usleep(10000);
                    ret = GpioGetInputVal(WIFI_IOT_GPIO_IDX_5, &keyValue);
                } while (ret == 0 && keyValue == WIFI_IOT_GPIO_VALUE0);
            }
        }
        
        // 自动模式下，自动进行布撤防
        Zdbcf();
        
        // 界面更新时，需要先删除界面显示的内容
        if(Now_Screen != Last_Screen)
        {
            Last_Screen = Now_Screen;
            OledFillScreen(0);
            Get_Status = GET_NORMAL;
            Last_Get_Status = GET_NORMAL;
            i=0;
        }

        // 获取时间和天气数据状态提示，会隔一段时间进行清除
        if((Get_Status == GET_FAIL) ||(Get_Status == GET_SUC))
        {
            i++;
            if(i > 20)
            {
                Get_Status = GET_NORMAL;
                i=0;

                switch(Now_Screen)
                {
                case TIMESCREEN:
                    OledClearString(16,7,"Sync time...",1);     //清除提示
                    break;
                case NOWSCREEN:
                case TOSCREEN:
                case ATOSCREEN:
                    printf("clear oled i = %d\n",i);
                    OledClearString(0,7,"Get Weather...",1);    //清除提示
                    break;
                case BCTIMESCREEN:
                    break;
                default:
                    break;
                }
            }
        }

        switch (Now_Screen)
        {
        case TIMESCREEN:
            TimeScreenDisp();       //显示时间显示界面
            DefendDisplay();
            break;

        case NOWSCREEN:
            WeatherDisp(NOWSCREEN); //显示现在的天气显示界面
            break;

        case TOSCREEN:              //显示明天天气显示界面
            WeatherDisp(TOSCREEN);
            break;

        case ATOSCREEN:             //显示后天天气显示界面
            WeatherDisp(ATOSCREEN);
            break;
            
        case BCTIMESCREEN:
            OLED_ShowCHinese(32,0,14);//时
            OLED_ShowCHinese(48,0,15);//间
            OLED_ShowCHinese(64,0,18);//设
            OLED_ShowCHinese(80,0,19);//值
            sprintf((char *)TimeStr,"%02d:00-%02d:00",starttime,stoptime);
            OledShowString(20,4,(char *)TimeStr,2); //显示布撤防时间
            break;
            
        case BCADJUSTSCEEN:
            OLED_ShowCHinese(32,0,16);//模
            OLED_ShowCHinese(48,0,17);//式
            OLED_ShowCHinese(64,0,18);//设
            OLED_ShowCHinese(80,0,19);//值
            DefendDisplay();          //显示手自动模式，布撤防状态
            break;
            
        default:
            break;
        }
        usleep(100000);
    }
}

static void OledDemo(void)
{
    osThreadAttr_t attr;

    attr.name = "OledTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    if (osThreadNew(OledTask, NULL, &attr) == NULL)
    {
        printf("[OledDemo] Falied to create OledTask!\n");
    }
}

APP_FEATURE_INIT(OledDemo);