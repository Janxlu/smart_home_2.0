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

//NTP服务器的IP和端口号
#define     NTPIPADDR       "52.231.114.183"
#define     NTPPORT         123

//1970与1900年时间戳差值
#define NTP_TIMESTAMP_DELTA 2208988800ull

//存放时间戳值
extern  uint32_t timedata;


//获取时间戳
bool getNtpTime(void)
{
    bool reflag = false;
    uint32_t NTP_Time;
    //存储NTP服务器返回的数据
    unsigned char buf[48];
    //时间请求需要的数据包
    unsigned char NTP_Data[]=
    {
        0x0b,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x0d6,0x06d,0x0d9,0x00,0x00,0x00,0x00,0x00
    }; 


    WifiDeviceConfig config = {0};

    // 准备AP的配置参数
    strcpy(config.ssid, PARAM_HOTSPOT_SSID);
    strcpy(config.preSharedKey, PARAM_HOTSPOT_PSK);
    config.securityType = PARAM_HOTSPOT_TYPE;
    osDelay(10);
    //连接到热点
    int netId = ConnectToHotspot(&config);

    ssize_t retval = 0;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket

    struct sockaddr_in toAddr = {0};
    toAddr.sin_family = AF_INET;
    toAddr.sin_port = htons(NTPPORT); // 端口号，从主机字节序转为网络字节序

    struct sockaddr_in fromAddr = {0};
    socklen_t fromLen = sizeof(fromAddr);

    if (inet_pton(AF_INET, NTPIPADDR, &toAddr.sin_addr) <= 0)   // 将主机IP地址从“点分十进制”字符串 转化为 标准格式（32位整数）
    {
        printf("inet_pton failed!\r\n");
        goto do_cleanup;
    }

    //设置1秒接收超时机制，超过一秒没收到数据，会返回
    struct timeval timeout= {1,0}; //1s
    if (setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(struct timeval)) == -1)
    {
        printf("setsockopt failed!\r\n");
        goto do_cleanup;
    }

    // UDP socket 是 “无连接的” ，因此每次发送都必须先指定目标主机和端口，主机可以是多播地址
    retval = sendto(sockfd, NTP_Data, 48, 0, (struct sockaddr *)&toAddr, sizeof(toAddr));
    if (retval < 0)
    {
        printf("sendto failed!\r\n");
        goto do_cleanup;
    }

    // UDP socket 是 “无连接的” ，因此每次接收时前并不知道消息来自何处，通过 fromAddr 参数可以得到发送方的信息（主机、端口号）
    retval = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&fromAddr, &fromLen);
    if (retval <= 0)
    {
        printf("recvfrom failed or abort, %ld, %d!\r\n", retval, errno);
        goto do_cleanup;
    }
    //时间数据获取
    NTP_Time = buf[40]<<24 | buf[40+1]<<16|buf[40+2]<<8 |buf[40+3];
    timedata = NTP_Time - NTP_TIMESTAMP_DELTA;

    printf("timedata is %ld\n",timedata);
    //response[retval] = '\0';
    printf("recv UDP message {%s} %ld done!\r\n", buf, retval);
    printf("peer info: ipaddr = %s, port = %d\r\n", inet_ntoa(fromAddr.sin_addr), ntohs(fromAddr.sin_port));
    reflag = true;

    for(uint8_t i = 0; i<48; i++)
    {
        printf("%02x\t",(unsigned char)buf[i]);
        if( (i+1) % 8 == 0 )
            printf("\r\n");
    }
do_cleanup:
    printf("do_cleanup...\r\n");
    close(sockfd);      //关闭socket

    printf("disconnect to AP ...\r\n");
    DisconnectWithHotspot(netId);       //断开与热点的连接
    printf("disconnect to AP done!\r\n");

    return reflag;
}