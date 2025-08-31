#ifndef __TCP_CONNECT_H
#define __TCP_CONNECT_H


#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "net_demo.h"
#include "net_common.h"
#include "net_params.h"
#include "wifi_connecter.h"
#include "ohos_init.h"
#include "cmsis_os2.h"

//远程服务器连接状态
typedef enum{
    DISCONNECTED = 0,   
    CONNECTED,
}TCP_STATIC;

extern TCP_STATIC connect_status;

//进行远程TCP连接
bool TcpConnect(void);
//断开与远程TCP的连接
bool TcpDisconnect(void);
//发送数据
bool TcpSend(char *data,int len);
//接收数据
bool TcpRev(void);

#endif  /*__TCP_CONNECT_H*/