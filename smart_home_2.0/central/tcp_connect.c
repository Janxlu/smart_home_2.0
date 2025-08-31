#include "tcp_connect.h"

//远程通信TCP服务器IP和端口号
#define     TCPIPADDR       "115.29.109.104"
#define     TCPPORT          6545

static int sockfd;
static int netId;
static struct sockaddr_in serverAddr = {0};

//存放与TCP服务器的连接状态
TCP_STATIC connect_status = DISCONNECTED;   //未连接

//进行远程TCP连接
bool TcpConnect(void)
{
    bool connectflag = false;
    WifiDeviceConfig config = {0};

    // 准备AP的配置参数
    strcpy(config.ssid, PARAM_HOTSPOT_SSID);
    strcpy(config.preSharedKey, PARAM_HOTSPOT_PSK);
    config.securityType = PARAM_HOTSPOT_TYPE;
    osDelay(10);
    netId= ConnectToHotspot(&config);

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket

    serverAddr.sin_family = AF_INET;  // AF_INET表示IPv4协议
    serverAddr.sin_port = htons(TCPPORT);  // 端口号，从主机字节序转为网络字节序
    if (inet_pton(AF_INET, TCPIPADDR, &serverAddr.sin_addr) <= 0)     // 将主机IP地址从“点分十进制”字符串 转化为 标准格式（32位整数）
    {
        printf("inet_pton failed!\r\n");
        goto do_cleanup;
    }

    //设置接收超时
    struct timeval timeout= {2,0}; //2s
    if (setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(struct timeval)) == -1)
    {
        printf("setsockopt failed!\r\n");
        //goto do_cleanup;
    }

    // 尝试和目标主机建立连接，连接成功会返回0 ，失败返回 -1
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        printf("connect failed!\r\n");
        goto do_cleanup;
    }
    printf("connect to server %s success!\r\n",  TCPIPADDR);
    connectflag = true;
    //连接状态为已连接
    connect_status = CONNECTED;
do_cleanup:
    return connectflag;
}

//断开与远程TCP的连接
bool TcpDisconnect(void)
{
    //关闭socket
    close(sockfd);
    //关闭连接的热点
    DisconnectWithHotspot(netId);
    connect_status = DISCONNECTED;  //未连接
    return true;
}

//发送到服务器
bool TcpSend(char *data,int len)
{
    int retval = send(sockfd, data, len, 0);
    if (retval < 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}
//存放接收的数据
extern char revData[30];
/*接收服务器的数据*/
bool TcpRev(void)
{
    int retval =0;
    //注意，已经设置有接收超时，没有数据返回时，不会一直阻塞
    retval = recv(sockfd, &revData, sizeof(revData), 0);
    if (retval <= 0)
    {
        return false;
    }
    revData[retval] = '\0';
    return true;
}