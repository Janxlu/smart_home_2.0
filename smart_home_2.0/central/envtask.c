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
#include "wifiiot_adc.h"
#include "wifiiot_errno.h"

#include "aht20.h"
#include "led.h"
#include "beep.h"

// 在 envtask.c 开头添加以下定义
typedef enum
{
    TIMESCREEN = 0,      // 时间显示界面，开机后的界面
    NOWSCREEN,           // 现在天气情况的显示界面
    TOSCREEN,            // 明天天气情况的显示界面
    ATOSCREEN,           // 后天天气情况的显示界面
    BCTIMESCREEN,        // 布撤防时间调整的显示界面
    BCADJUSTSCEEN,       // 设置手自动模式、布撤防状态的界面
    NOSCREEN,            // 无界面
} SCREEN_STATUS;

// 包含 Font 定义所需的头文件
#include "oled_ssd1306.h"

#define HUMAN_SENSOR_CHAN_NAME WIFI_IOT_ADC_CHANNEL_3
#define FLAME_SENSOR_GPIO WIFI_IOT_IO_NAME_GPIO_5  // 火焰传感器连接的GPIO

#define AHT20_BAUDRATE 400*1000
#define AHT20_I2C_IDX WIFI_IOT_I2C_IDX_0




// 添加外部变量声明（来自 oledtask.c）
extern SCREEN_STATUS Now_Screen;
extern SCREEN_STATUS Last_Screen;
extern void ShowFireAlarmScreen(void);
// 引入 OLED 相关函数
extern uint32_t OledInit(void);
extern void OledFillScreen(uint8_t fillData);
extern void OLED_ShowCHinese(uint8_t x, uint8_t y, uint8_t index);
extern void OledShowString(uint8_t x, uint8_t y, const char* str, Font font);
extern void ShowFireAlarmScreen(void); // 你刚封装的函数


extern uint8_t ywflag;      // 烟雾/火灾报警标志位
extern uint8_t rtflag;      // 防盗报警标志位
extern uint8_t bcflag;      // 布防状态标志位

int fire_alarm = 0;
unsigned short fire_state = 0; // 1，表示发生火灾，0表示未发生火灾
float humidity = 0.0f;         //用于保存湿度的变量
float temperature = 0.0f;      //用于保存温度的变量

void init(void)
{
    GpioInit();

    // 蜂鸣器引脚 设置为 PWM功能
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_PWM0_OUT);
    PwmInit(WIFI_IOT_PWM_PORT_PWM0);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_13, WIFI_IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_14, WIFI_IOT_IO_FUNC_GPIO_14_I2C0_SCL);
    I2cInit(AHT20_I2C_IDX, AHT20_BAUDRATE); //初始化I2c 的波特率

    //火焰接收器引脚
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_IO_FUNC_GPIO_5_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_GPIO_DIR_IN);
}

void EnvironmentTask(void *arg)
{
    (void)arg;
    init(); //初始化IIC
    OledInit(); // 初始化 OLED

    uint32_t retval = 0;
    uint16_t freqDivisor = 34052;
    unsigned short humandata = 0;
    int flameDetected = 0;
    static uint8_t last_screen_before_alarm = TIMESCREEN; // 保存报警前的界面
    

    // 发送初始化校准命令
    while (WIFI_IOT_SUCCESS != AHT20_Calibrate())
    {
        printf("AHT20 sensor init failed!\r\n");
        usleep(1000);
    }
    
    while (1)
    {
        // 发送 触发测量 命令，开始测量
        retval = AHT20_StartMeasure();
        if (retval != WIFI_IOT_SUCCESS)
        {
            printf("trigger measure failed!\r\n");
        }
        else
        {
            // 接收测量结果，拼接转换为标准值
            retval = AHT20_GetMeasureResult(&temperature, &humidity);
            printf("temp: %.2f,  humi: %.2f\n", temperature, humidity);
        }

        //火焰探测
        WifiIotGpioValue value;
        GpioGetInputVal(WIFI_IOT_IO_NAME_GPIO_5, &value); //获取火焰状态,0表示探测到火焰，1表示未探测到

        //获取到的值转化为火灾状态
        fire_state = value ? 0 : 1;
        flameDetected = fire_state;

        if (fire_state)
        {
            // 如果是第一次检测到火灾，保存当前界面
            if (!fire_alarm) {
                last_screen_before_alarm = Now_Screen; // 保存当前显示界面
                
            }
            fire_alarm = 1;
            ywflag = 1; // 设置火灾报警标志
            PwmStart(WIFI_IOT_PWM_PORT_PWM0, freqDivisor / 20, freqDivisor);
            ShowFireAlarmScreen(); // 显示火灾报警界面
        }
        else
        {
            if (fire_alarm) {
                // 火灾刚结束，恢复之前的界面
                fire_alarm = 0;
                ywflag = 0;
                PwmStop(WIFI_IOT_PWM_PORT_PWM0);
                
                // 恢复报警前的界面
                Now_Screen = last_screen_before_alarm;
                Last_Screen = NOSCREEN; // 强制刷新界面
                OledFillScreen(0x00); // 清屏准备重绘
            }
        }

        //防盗检测
        if(bcflag)      //在布防状态下，防盗检测功能才开启
        {
            if (AdcRead(HUMAN_SENSOR_CHAN_NAME, &humandata, WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0)
                    == WIFI_IOT_SUCCESS)
            {
                //布防功能开启下，检测到人。
                if((humandata > 1500) && (rtflag == 0))
                {
                    rtflag = 1;     //置位防盗报警标志位
                    Beep_Start();   //蜂鸣器发声报警
                    printf("Intruder detected! Security alarm triggered!\r\n");
                }
            }
        }
        else
        {
            //切换到撤防状态，复位防盗报警标志位
            if((rtflag == 1) && (ywflag == 0))  //之前触发防盗报警且没有火灾报警
                Beep_Stop();
            rtflag = 0;
        }

        // 系统心跳灯
        Led_Set(GREEN, ON);      //系统心跳灯开
        sleep(1);
        Led_Set(GREEN, OFF);     //系统心跳灯关
        sleep(1);
        
        // 打印状态信息
        printf("Temperature: %.1f°C, Humidity: %.1f%%, Flame: %s, Fire Alarm: %s, Security Alarm: %s\r\n",
               temperature, humidity,
               flameDetected ? "DETECTED" : "NORMAL",
               ywflag ? "ON" : "OFF",
               rtflag ? "ON" : "OFF");
    }
}

static void EnvironmentDemo(void)
{
    osThreadAttr_t attr;
    attr.name = "EnvironmentTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    if (osThreadNew(EnvironmentTask, NULL, &attr) == NULL)
    {
        printf("[EnvironmentDemo] Falied to create EnvironmentTask!\n");
    }
}

APP_FEATURE_INIT(EnvironmentDemo);