#ifndef _DRIVERS_H
#define _DRIVERS_H

#include "bsp.h"
#include "driver_misc.h"
#include "usart.h"
#include "w25p80.h"
#include "sdcard.h"

 //下面是获取到串口1的数据
#define BUFLEN 80
typedef struct _UART_BUF
{
    char buf [BUFLEN+1];                // 多申请一个字符保证最后一个字符永远是'0'
    unsigned int index ;
}UART_BUF;


typedef enum {
    TCPSERVER,
    TCPCLIENT,
    UDPSERVER,
    UDPCLIENT

}SOCKET_STATUS ;

extern SOCKET_STATUS i_socket;

#endif

