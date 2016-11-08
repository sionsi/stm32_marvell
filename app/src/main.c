#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"

#include "test.h"
#include "dhcpd.h"
#include "webserver.h"
#include <cctype>
#include "delay.h"
#include "DHT11_BUS.h"
#include "oled.h"
#include "bmp.h"
#define VIRSION		"V8.8"

wait_event_t command_event = 0;		//单个字节命令事件 usart.c					//单个字节命令
extern void clear_buf_uart1(void);	//清空串口1数据
extern UART_BUF buf_uart1;			//串口的接收数据
SOCKET_STATUS i_socket = TCPSERVER;  //默认板子是TCP服务器
extern test_create_adhoc(void);
uint8_t Data_Array[5];				//温湿度数组

void choose_socket(char * tmp)
{
	if(strcmp(tmp, "1001")==0) 
	{
		i_socket = TCPSERVER;
		test_close_tcp_server();
		test_tcp_server();
	}
	if(strcmp(tmp, "1003")==0) 
	{
		i_socket = UDPSERVER;
		test_close_tcp_server();
		test_udp_server();
	}
	if(strcmp(tmp, "1000")==0) 
	{
		i_socket = TCPCLIENT;
		test_tcp_unlink();
		test_tcp_link();
	}
	if(strcmp(tmp, "1002")==0) 
	{
		i_socket = UDPCLIENT;
		test_udp_link();
	}
}

void handle_cmd(char *cmd)
{
		p_dbg(cmd);
		if(strcmp(cmd, "AT+")==0)
			show_sys_info();

		if(strcmp(cmd, "AT+INFO")==0)
			show_sys_info();


		if(strcmp(cmd, "AT+SCAN")==0)
			test_scan();


		if(strcmp(cmd, "AT+CONN")==0) 
			test_wifi_connect();

		if(strcmp(cmd, "AT+DISCONN")==0) 
			test_wifi_disconnect();

		if(strcmp(cmd, "AT+AP")==0) 
			test_create_ap();

		if(strcmp(cmd, "AT+TCP")==0) 
			test_tcp_link();

		if(strcmp(cmd, "AT+DISTCP")==0) 
			test_tcp_unlink();

		if(strcmp(cmd, "AT+DNS")==0) 
			test_dns("www.baidu.com");
		
		if(strcmp(cmd, "AT+GETIP")==0) 
			test_auto_get_ip();

		if(strcmp(cmd, "AT+SLID")==0) 
#if USE_MEM_DEBUG	
			mem_slide_check(1);
#endif

		if(strcmp(cmd, "AT+STATUS")==0) 
			test_wifi_get_stats();


		if(strcmp(cmd, "AT+UDP")==0) 
			test_udp_link();


		if(strcmp(cmd, "AT+DBG")==0) 
			switch_dbg();
		
		if(strcmp(cmd, "AT+DISAP")==0) 
			test_stop_ap();

		if(strcmp(cmd, "AT+LIST")==0) 
			test_get_station_list();
		
		if(strcmp(cmd, "AT+ADHOC")==0) 
			test_wifi_join_adhoc();

		if(strcmp(cmd, "AT+DISADHOC")==0) 
			test_wifi_leave_adhoc();

		if(strcmp(cmd, "AT+PWR")==0) 
			test_power_save_enable();
		
		if(strcmp(cmd, "AT+DISPWR")==0) 
			test_power_save_disable();
			

		if(strcmp(cmd, "AT+MONITOR")==0) 
                        ;//monitor_switch();
		
		if(strcmp(cmd, "AT+FULL")==0) 
			test_full_speed_send();
		
		if(strcmp(cmd, "AT+DISFULL")==0) 
			test_full_speed_send_stop();
		
		if(strcmp(cmd, "AT+BACK")==0)  
			switch_loopback_test();
		
		if(strcmp(cmd, "AT+RST")==0) 
			soft_reset();
		

#ifndef FW_OUTSIDE
		if(strcmp(cmd, "AT+FIRMWARE")==0) 
			test_write_firmware_to_spi_flash();
#endif

		if(strcmp(cmd, "AT+TCPSERVER")==0) 
		{
			i_socket = TCPSERVER;
			test_close_tcp_server();
			test_tcp_server();
		}
		if(strcmp(cmd, "AT+UDPSERVER")==0) 
		{
			i_socket = UDPSERVER;
			test_close_tcp_server();
			test_udp_server();
		}
		if(strcmp(cmd, "AT+TCPCLIENT")==0) 
		{
			i_socket = TCPCLIENT;
			test_tcp_unlink();
			test_tcp_link();
		}
		if(strcmp(cmd, "AT+UDPCLIENT")==0) 
		{
			i_socket = UDPCLIENT;
			test_udp_link();
		}

		p_dbg("OK\r\n");
}


void main_thread(void *pdata)
{
	int ret;
	#ifdef DEBUG
	RCC_ClocksTypeDef RCC_ClocksStatus;
	#endif
	
	delay_init(72);				//延时初始化
	
   	OLED_Init();				//初始化OLED  
	OLED_Clear(); 
	OLED_DrawBMP(0,0,128,8,BMP1);  //图片显示(图片显示慎用，生成的字表较大，会占用较多空间，FLASH空间8K以下慎用)
	OLED_ShowCHinese(32,6,0);//创
	OLED_ShowCHinese(50,6,1);//思
	OLED_ShowCHinese(68,6,2);//通
	OLED_ShowCHinese(86,6,3);//信

	DHT11Read(Data_Array);		//获取温湿度

	driver_misc_init(); 		//初始化一些杂项(驱动相关)
	usr_gpio_init(); 			//初始化GPIO
    D2(0);						//亮

	OSStatInit(); 				//初始化UCOS状态
	uart1_init(); 				//初始化串口1

	//打印MCU总线时钟
	#ifdef DEBUG
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	#endif
	p_dbg("SYSCLK_Frequency:%d,HCLK_Frequency:%d,PCLK1_Frequency:%d,PCLK2_Frequency:%d,ADCCLK_Frequency:%d\n", 
		RCC_ClocksStatus.SYSCLK_Frequency, 
		RCC_ClocksStatus.HCLK_Frequency, 
		RCC_ClocksStatus.PCLK1_Frequency, 
		RCC_ClocksStatus.PCLK2_Frequency, 
		0);
#if !defined(CHIP_F103) || defined(FW_OUTSIDE)
	//F103芯片驱动时，如果没有外部flash，不要执行这里
	m25p80_init(); //初始化SPI-flash
#endif

	init_work_thread();	//初始化工作线程

	ret = SD_Init();    //初始化SDIO设备
		
	if (ret == 0){
		ret = init_wifi();//初始化WIFI芯片
	}
	
	if(ret != 0)
	{
		p_err("init wifi faild!");  //wifi初始化失败，不能往下走了
		while(1);
	}

	init_monitor(); //初始化monitor模块,必须在init_wifi之后调用

	init_lwip(); //初始化lwip协议栈

	init_udhcpd(); //初始化dhcp服务器

	enable_dhcp_server(); // 开启dhcp服务器,如果工作在sta模式,可以不开启dhcpserver

	misc_init(); //初始化一些杂项(app相关)

	command_event = init_event(); //初始化一个等待事件

/*
 *下面启动开发板预设的功能
 *开启web服务器, 端口号:80
 *开启TCP服务器，端口号:1001
 *开启AP功能（名称：WIFIBORAD_AP，密码：12345678）
 *开启低功耗（模块不会发热）
*/
#if	SUPPORT_WEBSERVER
	web_server_init();
#endif
	switch(i_socket)
	{
		case TCPSERVER:
		test_tcp_server();
		break;
	    case TCPCLIENT:
		break;
	    case UDPSERVER:
		test_udp_server();
		break;
	    case UDPCLIENT:
		break;
		default :
		break;
	}
	OLED_Clear();

	D2(1);LED1(1);LED2(1);LED3(1);RELAY(1);			   //灭
	
	p_dbg("CSIC代码版本:%s", VIRSION);
	p_dbg("CSIC启动时间%d.%d S", os_time_get() / 1000, os_time_get() % 1000);
	p_dbg("sensen test");
	p_dbg("所有驱动协议栈初始化完毕");

	test_create_ap();
	test_power_save_enable();  //开启低功耗（模块不会发热）
	//test_wifi_connect();

	while (1)
	{
		//等待用户命令
		wait_event(command_event);

		//执行命令
		if(strstr(buf_uart1.buf,"AT+") !=0 )
		{
        	handle_cmd(buf_uart1.buf);
			clear_buf_uart1();
		}

		if(strlen(buf_uart1.buf)>0)
		{
			if(socket_init_ok())
			{
				p_dbg("SEND %s ",buf_uart1.buf);
				switch(i_socket)
				{
					case TCPSERVER:
						test_send(buf_uart1.buf);
						clear_buf_uart1();
					break;
				    case TCPCLIENT:
						test_send(buf_uart1.buf);
						clear_buf_uart1();
					break;
				    case UDPSERVER:
						test_sendto(buf_uart1.buf);
						clear_buf_uart1();
					break;
				    case UDPCLIENT:
						test_send(buf_uart1.buf);
						clear_buf_uart1();
					break;
					default :
					break;
				}
			}
		}
	}
}

int main(void)
{	
	OSInit();
	_mem_init(); //初始化内存分配

	thread_create(main_thread, 0, TASK_MAIN_PRIO, 0, TASK_MAIN_STACK_SIZE, "main_thread");

	OSStart();
	return 0;
}
