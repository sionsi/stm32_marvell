#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"

#include "test.h"
#include "lwip/netif.h"
#include <absacc.h>


void delay_1us()
{
	int i = 25;
	while (i--)
		;
}

void delay_us(uint32_t us)
{
	while (us--)
		delay_1us();
}


void assert_failed(uint8_t *file, uint32_t line)
{
	p_err("assert_failed in:%s,line:%d \n", file ? file : "n", line);
	while (1);
}

void misc_init()
{
}

void show_sys_info()
{
	p_dbg("===========================");
	p_dbg("本板子的IP信息 如下：");
	p_dbg("mac:%02x-%02x-%02x-%02x-%02x-%02x", p_netif->hwaddr[0], p_netif->hwaddr[1], p_netif->hwaddr[2], p_netif->hwaddr[3], p_netif->hwaddr[4], p_netif->hwaddr[5]);
	show_tcpip_info();

	switch(i_socket)
	{
		case TCPSERVER:
		p_dbg("socket:TCPSERVER");	
		break;
	    case TCPCLIENT:
		p_dbg("socket:TCPCLIENT");		
		break;
	    case UDPSERVER:
		p_dbg("socket:UDPSERVER");		
		break;
	    case UDPCLIENT:
		p_dbg("socket:UDPCLIENT");		
		break;
		default :
		break;
	}
	p_dbg("注意：UDP需要客户端先发数据到服务器");
	p_dbg("===========================");
}

void soft_reset()
{
	p_err("system well reset\n");
	NVIC_SystemReset();
  	__DSB();  
	while (1);
}

void HardFault_Handler()
{
#if USE_MEM_DEBUG		
	static int once = 0;
	if (!once)
	{
		once = 1;

		mem_slide_check(0);

	}
#endif
	p_err("系统错误:%s\n", __FUNCTION__);
	while (1)
		;
}


#if OS_APP_HOOKS_EN > 0u
void App_TaskCreateHook(OS_TCB *ptcb)
{
	ptcb = ptcb;
}

void App_TaskDelHook(OS_TCB *ptcb)
{
	ptcb = ptcb;
}

void App_TaskReturnHook(OS_TCB *ptcb)
{
	ptcb = ptcb;
}

void App_TCBInitHook(OS_TCB *ptcb)
{
	ptcb = ptcb;
}

void App_TaskSwHook(void){

}

void App_TimeTickHook(void){}

//uC/OS-II Stat线程中调用此函数，每100MS一次
void App_TaskStatHook()
{
	#if USE_MEM_DEBUG
	mem_slide_check(0);
	#endif
        if (is_wifi_connected())	 //连接成功亮 否则灭
            D1(0);
        else
            D1(1);

        if(!GPIO_ReadInputDataBit(BUTTON_PORT, BUTTON_PIN))	    /*检测D2数据，是否为1*/
        {
          	sleep(100);
          	p_dbg("key down");											/*返回KEY2*/
		  	switch(i_socket)
			{
				case TCPSERVER:
				break;
			    case TCPCLIENT:
				test_tcp_unlink();
				test_tcp_link();
				break;
			    case UDPSERVER:
				break;
			    case UDPCLIENT:
				test_udp_link();
				break;
				default :
				break;
			}
        }
}

#endif
extern uint32_t drvdbg;
extern uint32_t drvdbg_save;
void switch_dbg(void)
{
	if(dbg_level)
	{
		p_dbg("close dbg");
	}
	dbg_level = !dbg_level;
	if(dbg_level)
	{
		p_dbg("open dbg");
#ifdef DEBUG_LEVEL1
		drvdbg = drvdbg_save;
#endif
	}
#ifdef DEBUG_LEVEL1	
	else
		drvdbg = 0;
#endif
}

void usr_gpio_init()
{

    GPIO_InitTypeDef GPIO_InitStructure;
    /* D1 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* D2 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    /* 继电器 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

	PWR_BackupAccessCmd(ENABLE);//允许修改RTC 和后备寄存器
    RCC_LSEConfig(RCC_LSE_OFF);//关闭外部低速外部时钟信号功能 PC14 PC15 才可以当普通IO用。

    /* LED1 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* LED2 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    /* LED3 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);


    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = BUTTON_PIN;
    GPIO_Init(BUTTON_PORT, &GPIO_InitStructure);


}


