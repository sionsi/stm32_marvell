#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"

#include <absacc.h>
extern char Image$$RW_IRAM1$$ZI$$Limit[];
void _mem_init(void)
{
	uint32_t malloc_start, malloc_size;

	malloc_start = (uint32_t) &Image$$RW_IRAM1$$ZI$$Limit; //取空闲内存最低地址
	malloc_start = (malloc_start + 3) & (~0x03);// ALIGN TO word

//0x20000为板子内存总大小，更换MCU时需注意
	malloc_size = 0x20000000 + 0x10000 - malloc_start;

	sys_meminit((void*)malloc_start, malloc_size);
}

void init_rng()
{

}

void init_systick()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	SysTick_Config(SystemFrequency / (OS_TICKS_PER_SEC *10));
}

/**
 *读一个随机数
 */
uint32_t get_random(void)
{
	return os_time_get();
}

void gpio_cfg(uint32_t group, uint32_t pin, uint8_t mode)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = pin;
	GPIO_InitStructure.GPIO_Mode = (GPIOMode_TypeDef) mode;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init((GPIO_TypeDef*)group, &GPIO_InitStructure); 
}

void driver_gpio_init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);
}


int check_rst_stat()
{
	uint32_t stat;
	stat = RCC->CSR;
	RCC->CSR = 1L << 24; //清除复位标志

	p_err("reset:");
	if (stat &(1UL << 31))
	// 低功耗复位
	{
		p_err("low power\n");
	}
	if (stat &(1UL << 30))
	//窗口看门狗复位
	{
		p_err("windw wdg\n");
	}
	if (stat &(1UL << 29))
	//独立看门狗复位
	{
		p_err("indep wdg\n");
	}
	if (stat &(1UL << 28))
	//软件复位
	{
		p_err("soft reset\n");
	}
	if (stat &(1UL << 27))
	//掉电复位
	{
		p_err("por reset\n");
	}
	if (stat &(1UL << 26))
	//rst复位
	{
		p_err("user reset\n");
	}

	return 0;
}

void driver_misc_init()
{
	driver_gpio_init();
	init_rng();
	init_systick();
}

