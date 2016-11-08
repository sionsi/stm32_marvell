#ifndef _DHT11_BUS_H_
#define _DHT11_BUS_H_
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "delay.h"
//数据IO口选择
#define GPIO           GPIOA
#define RCC_IO         RCC_APB2Periph_GPIOA
#define PIN            GPIO_Pin_1
//读取IO数组
#define Read           GPIO_ReadInputDataBit(GPIO,PIN)
//a=1:输出高电平
//a=0:输出低电平
#define OutPut(a)  if(a)\
						GPIO_SetBits(GPIO,PIN); \
					else \
						GPIO_ResetBits(GPIO,PIN)			

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<															
/*数据显示格式：
*             湿度：byte4.byte3=？？（%RH）
*             温度：byte2.byte1=？？（℃）
*             校验：byte4+byte3+byte2+byte1=（温度+湿度）
*输入一个一维数组指针，
*结果：改变该数组的值
*             值一：湿度整数
*             值二：湿度小数
*             值三：温度整数
*             值四：温度小数
*             值五：校验和
*/															
void DHT11Read(uint8_t*Data_Array);
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>			
																						
//内部调用
static void DHT11WriteStart(void);//引导信号
static uint8_t  Byte_Receive(void);//数据接收
static void Input_GPIO_Config(void);//IO口接收配置
static void Output_GPIO_Config(void);//IO口发送配置
#endif
