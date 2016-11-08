#ifndef _MISC_H
#define _MISC_H

#define D1(tmp) GPIOB->ODR=tmp?(GPIOB->ODR|1<<7):(GPIOB->ODR&(~(1<<7)))
#define D2(tmp) GPIOB->ODR=tmp?(GPIOB->ODR|1<<6):(GPIOB->ODR&(~(1<<6)))
#define RELAY(tmp) GPIOA->ODR=tmp?(GPIOA->ODR|1<<4):(GPIOA->ODR&(~(1<<4)))

#define LED1(tmp) GPIOC->ODR=tmp?(GPIOC->ODR|1<<15):(GPIOC->ODR&(~(1<<15)))
#define LED2(tmp) GPIOC->ODR=tmp?(GPIOC->ODR|1<<13):(GPIOC->ODR&(~(1<<13)))
#define LED3(tmp) GPIOB->ODR=tmp?(GPIOB->ODR|1<<9):(GPIOB->ODR&(~(1<<9)))

#define MAR_POW(tmp) GPIOB->ODR=tmp?(GPIOB->ODR|1<<12):(GPIOB->ODR&(~(1<<12)))




#define BUTTON_PORT		GPIOB
#define BUTTON_PIN		GPIO_Pin_14
#define BUTTON_SOURCE		GPIO_PinSource1
#define BUTTON_EXTI_LINE 	EXTI_Line1
#define BUTTON_STAT 	GPIO_STAT(BUTTON_PORT, BUTTON_PIN)


void usr_gpio_init(void);
void init_systick(void);
void misc_init(void);

void soft_reset(void);
void delay_1us(void);
void delay_us(uint32_t us);

#endif
