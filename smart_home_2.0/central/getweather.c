#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "net_demo.h"
#include "net_common.h"
#include "net_params.h"
#include "wifi_connecter.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "cjsonparse.h"

//获取天气数据的服务器IP和端口号
#define     WEATHERIPADDR       "116.62.81.138"
#define     WEATHERPORT          80

//最近三天数据的请求数据包
static char requestday[] = "GET https://api.seniverse.com/v3/weather/daily.json?key=SgJs9V9ghopE5WSBe&location=shenzhen&language=zh-Hans&unit=c&start=0&days=3\r\n\r\n";
//现在天气情况的请求数据包
static char requestnow[] = "GET https://api.seniverse.com/v3/weather/now.json?key=SgJs9V9ghopE5WSBe&location=shenzhen&language=zh-Hans&unit=c\r\n\r\n";

//存放获取到的数据包
static char response[1200] = "";


weather weatherValue;

//获取天气数据情况，包括最近三天的天气数据和现在的天气情况
bool getWeather(void)
{
    bool sucflag = false;
    WifiDeviceConfig config = {0};

    // 准备AP的配置参数
    strcpy(config.ssid, PARAM_HOTSPOT_SSID);
    strcpy(config.preSharedKey, PARAM_HOTSPOT_PSK);
    config.securityType = PARAM_HOTSPOT_TYPE;
    osDelay(10);
    //连接到热点
    int netId = ConnectToHotspot(&config);

    /*获取最近三天天气情况*/
    int32_t retval = 0;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;  // AF_INET表示IPv4协议
    serverAddr.sin_port = htons(WEATHERPORT);  // 端口号，从主机字节序转为网络字节序
    if (inet_pton(AF_INET, WEATHERIPADDR, &serverAddr.sin_addr) <= 0)    // 将主机IP地址从“点分十进制”字符串 转化为 标准格式（32位整数）
    {
        printf("inet_pton failed!\r\n");
        goto do_cleanup;
    }
    // 尝试和目标主机建立连接，连接成功会返回0 ，失败返回 -1
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        printf("connect failed!\r\n");
        goto do_cleanup;
    }
    printf("connect to server %s success!\r\n", WEATHERIPADDR);

    // 建立连接成功之后，这个TCP socket描述符 —— sockfd 就具有了 “连接状态”，发送、接收 对端都是 connect 参数指定的目标主机和端口
    //发送最近三天天气情况数据请求包到服务器
    retval = send(sockfd, requestday, sizeof(requestday), 0);
    if (retval < 0)
    {
        printf("send request failed!\r\n");
        goto do_cleanup;
    }
    printf("send request{%s} %ld to server done!\r\n", requestday, retval);
    //数据接收
    retval = recv(sockfd, &response, sizeof(response), 0);
    if (retval <= 0)
    {
        printf("send response from server failed or done, %ld!\r\n", retval);
        goto do_cleanup;
    }
    response[retval] = '\0';
    printf("rev response from server  %ld!\r\n", retval);
    int i = 0;
    /*打印接收到数据*/
    while(i<retval)
    {
        printf("%c",response[i]);
        i++;
    }
    //解析最近三天的天气数据情况
    cJSON_TayWeatherParse(response,&weatherValue);
    //关闭socket
    close(sockfd);

    /*获取现在的天气情况*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    if (inet_pton(AF_INET, WEATHERIPADDR, &serverAddr.sin_addr) <= 0)    // 将主机IP地址从“点分十进制”字符串 转化为 标准格式（32位整数）
    {
        printf("inet_pton failed!\r\n");
        goto do_cleanup;
    }
    // 尝试和目标主机建立连接，连接成功会返回0 ，失败返回 -1
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        printf("connect failed!\r\n");
        goto do_cleanup;
    }
    //发送现在的天气情况数据请求包到服务器
    retval = send(sockfd, requestnow, sizeof(requestnow), 0);
    if (retval < 0)
    {
        printf("send request failed!\r\n");
        goto do_cleanup;
    }
    printf("send request{%s} %ld to server done!\r\n", requestnow, retval);
    //接收数据
    retval = recv(sockfd, &response, sizeof(response), 0);
    if (retval <= 0)
    {
        printf("send response from server failed or done, %ld!\r\n", retval);
        goto do_cleanup;
    }
    response[retval] = '\0';
    i = 0;
    /*打印接收到数据*/
    while(i<retval)
    {
        printf("%c",response[i]);
        i++;
    }
    //解析现在天气数据
    cJSON_NowWeatherParse(response,&weatherValue);
    sucflag=true;
do_cleanup:
    close(sockfd);                  //关闭socket
    DisconnectWithHotspot(netId);   //端口与热点的连接
    if(sucflag)
        return true;
    else
        return false;
}