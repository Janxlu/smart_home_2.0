#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include "tcp_connect.h"
#include "ohos_init.h"
#include "cmsis_os2.h"

//发送到手机端的数据格式
char sendData[]="T:26H:96Z:0B:0D:0Y:0";
//存放接收到的数据
char revData[30]=""; 

//获取状态枚举
typedef enum{
    GET_NORMAL =0 ,
    GET_PROPRESS,
    GET_SUC,
    GET_FAIL,
}GET_STATUS;

extern GET_STATUS Get_Status;
extern float humidity;
extern float temperature;
extern uint8_t bcflag ;
extern uint8_t zdflag ;
extern uint8_t ywflag ;
extern uint8_t rtflag ;

extern bool getNtpTime(void);
extern bool getWeather(void);

//数据发送任务
static void TcpSendTask(void *arg)
{
    (void)arg;
    sleep(3);
    //先获取时间和天气数据
    getNtpTime();
    getWeather();
    uint8_t i = 0;

    //尝试与与远程服务器进行连接
    while(1){
        if(TcpConnect())
        {
            printf("Tcp Connect Suc\n");
            break;
        }
        else{
            i++;
        }
        if(i>10)    //超过十次则视为连接不成功，跳出
        break;
    }
    if(i>10)
        printf("Tcp Connect fail\n");

    while(1)
    {
        
        if(connect_status == CONNECTED)
        {
            //发送数据到远程服务器
            sprintf(sendData,"T:%02dH:%02dZ:%01dB:%01dD:%01dY:%01d",
            (uint32_t)temperature,(uint32_t)humidity,(uint32_t)zdflag,(uint32_t)bcflag,(uint32_t)rtflag,(uint32_t)ywflag);
            //如果发送不成功，则视为连接已经断开
            if(!TcpSend(sendData,sizeof(sendData)-1)){
                connect_status = DISCONNECTED;
                TcpDisconnect();    //主动断开与远程TCP服务器的连接
            }
        }
        else{
                //在与远程服务器为未连接状态，重新建立连接
                if(Get_Status == GET_NORMAL){   //没有在获取时间或者天气数据，则重新尝试与远程服务器建立连接
                    if(TcpConnect()){
                        printf("Tcp Connect Suc\n");
                    }
                }
        }
        sleep(2);
    }
}

static void TcpSendTaskHandle(void)
{
    osThreadAttr_t attr;
    attr.name = "TcpSendTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;
    if (osThreadNew(TcpSendTask, NULL, &attr) == NULL) {
        printf("[TcpSendTaskHandle] Falied to create TcpSendTask!\n");
    }
}
APP_FEATURE_INIT(TcpSendTaskHandle);

//远程服务器数据接收任务
static void TcpRevTask(void *arg){
    (void)arg;
    while(1)
    {
        //为已连接状态，接收数据
        if(connect_status == CONNECTED)
        {
            if(TcpRev()){
                //成功接收到手机端发送的数据
                if(strcmp(revData,"s")==0)          //设置手动模式
                {
                    zdflag = 0;
                }
                else if(strcmp(revData,"z")==0)     //设置自动模式
                {
                    zdflag = 1;
                }
                else if(strcmp(revData,"b")==0)     //设置布防状态
                {
                    bcflag =1;
                }
                else if(strcmp(revData,"c")==0)     //设置撤防状态
                {
                    bcflag =0;
                }
            }
        }
        usleep(10000);
    }
}

static void TcpRevTaskHandle(void)
{
    osThreadAttr_t attr;
    attr.name = "TcpRevTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;
    if (osThreadNew(TcpRevTask, NULL, &attr) == NULL) {
        printf("[TcpRevTaskHandle] Falied to create TcpRevTask!\n");
    }
}
APP_FEATURE_INIT(TcpRevTaskHandle);