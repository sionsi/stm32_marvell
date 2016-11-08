//#define DEBUG
#include "drivers.h"
#include "app.h"
#include "api.h"


UART_RECV 	u1_recv;

UART_SEND 	u3_send;

UART_SEND 	u1_send;

wait_event_t uart1_rx_waitq = 0;

volatile unsigned int uart1_cnt = 0,uart1_cnt_ex = 0;


/*
 * 函数名：USART1_Config
 * 描述  ：USART1 GPIO 配置,工作模式配置。115200 8-N-1
 * 输入  ：无
 * 输出  : 无
 * 调用  ：外部调用
 */
void uart1_init(void)
{
	USART_InitTypeDef USART_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
	
	memset(&u1_recv,0,sizeof(UART_RECV));
	
	u1_recv.pkg.pending.val = 1;  //防止用户输入
	
//	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); 
//	RCC_AHB1PeriphClockCmd(RCC_AHBPeriph_DMA2|RCC_APB1Periph_USART2,ENABLE);
//
//	gpio_cfg((uint32_t)UART1_TX_PORT_GROUP, UART1_TX_PIN, GPIO_Mode_AF_PP);
//	gpio_cfg((uint32_t)UART1_RX_PORT_GROUP, UART1_RX_PIN, GPIO_Mode_AF_IF);
////	GPIO_PinAFConfig(UART1_TX_PORT_GROUP, UART1_TX_PIN_SOURSE, GPIO_AF_USART1);
////	GPIO_PinAFConfig(UART1_RX_PORT_GROUP, UART1_RX_PIN_SOURSE, GPIO_AF_USART1);
//
//	GPIO_PinAFConfig(UART1_TX_PORT_GROUP, UART1_TX_PIN_SOURSE, GPIO_AF_USART2);
//	GPIO_PinAFConfig(UART1_RX_PORT_GROUP, UART1_RX_PIN_SOURSE, GPIO_AF_USART2);

	/* config USART1 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	//RCC_APB1PeriphClockCmd(RCC_APB1ENR_USART3EN|RCC_APB2Periph_USART1,ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	/* USART1 GPIO config */
   /* Configure USART1 Tx (PA.09) as alternate function push-pull */
	gpio_cfg((uint32_t)GPIOA, GPIO_Pin_9, GPIO_Mode_AF_PP);
    /* Configure USART1 Rx (PA.10) as input floating */
	gpio_cfg((uint32_t)GPIOA, GPIO_Pin_10, GPIO_Mode_IN_FLOATING);

	
	u1_send.wait = init_event(); //初始化串口信号量u1_send.wait
	if(u1_send.wait  == 0)
	{
		p_err("uart_init sys_sem_new1 err\n");
		return;
	}

	USART_InitStructure.USART_BaudRate = UART1_DEFAULT_BAUD;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure); 
	USART_Cmd(USART1, ENABLE);
	//USART_ITConfig(USART1, USART_IT_TC, ENABLE);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
//	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USART1_IRQn_Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	USART_ClearITPendingBit(USART1, USART_IT_TC);
	USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	NVIC_EnableIRQ(USART1_IRQn);

}


void uart3_init(void)
{

}



int fputc(int ch, FILE *f)
{
	uint32_t timer = 100*1000, irq_flag;
	while(u1_send.sending.val)   //循环等待
	{
		if(timer-- == 0)
			break;
	}

	irq_flag = local_irq_save();
	if(!u1_send.sending.val)
		u1_send.sending.val = TRUE;
	else{
		local_irq_restore(irq_flag);
		return -1;
	}
	//while(!(USART1->SR & USART_FLAG_TXE));
	//USART1->DR = ch;
	while(!(USART1->SR & USART_FLAG_TXE));
	USART1->DR = ch;
	u1_send.sending.val = FALSE;
	local_irq_restore(irq_flag);
	return (ch);
}

int uart3_send_next()
{
	if(u3_send.cnt < u3_send.size)
	{

		USART3->DR = u3_send.buff[u3_send.cnt++];
		return 0;
	}
	else
	{
		USART3->SR = ~ ((uint16_t)((uint16_t)0x01 << 6));
		u3_send.sending.val = 0;    
		wake_up(u3_send.wait);
		return 1;
	}
}

int uart3_send(u8_t *buff,u32_t size)
{
	uint32_t irq_flag;
	int ret;
	if((size == 0) || (buff == 0))
		return -1;
	irq_flag = local_irq_save(); 
	while(!(USART3->SR & USART_FLAG_TXE));
	u3_send.size = size;
	u3_send.cnt = 0;
	u3_send.buff = buff;
	u3_send.sending.val = 1;
	local_irq_restore(irq_flag);
	uart3_send_next();
	ret = wait_event_timeout(u3_send.wait, 2000);
	u3_send.sending.val = 0;  
	if((-1 == ret) || (u3_send.cnt  !=  u3_send.size))
	{
		p_err("uart2_send err:%d,%d,%d\n",ret,u3_send.cnt,u3_send.size);
	}
	return 0;
}


int uart1_send_next()
{
	if(u1_send.cnt < u1_send.size)
	{

		USART1->DR = u1_send.buff[u1_send.cnt++];
		return 0;
	}
	else
	{
		USART1->SR = ~ ((uint16_t)((uint16_t)0x01 << 6));
		USART_ITConfig(USART1, USART_IT_TC, DISABLE);
		u1_send.sending.val = 0;    
		wake_up(u1_send.wait);
		return 1;
	}
}

int uart1_send(u8_t *buff,u32_t size)
{
	uint32_t irq_flag;
	int ret;
	if((size == 0) || (buff == 0))
		return -1;
	irq_flag = local_irq_save(); 
	while(!(USART1->SR & USART_FLAG_TXE));
	u1_send.size = size;
	u1_send.cnt = 0;
	u1_send.buff = buff;
	u1_send.sending.val = 1;
	local_irq_restore(irq_flag);
	USART_ClearFlag(USART1, USART_FLAG_TC);
	USART_ITConfig(USART1, USART_IT_TC, ENABLE);
	uart1_send_next();
	ret = wait_event_timeout(u1_send.wait, 2000);
	u1_send.sending.val = 0;  
	if((-1 == ret) || (u1_send.cnt  !=  u1_send.size))
	{
		p_err("uart1_send err:%d,%d,%d\n",ret,u1_send.cnt,u1_send.size);
	}
	return 0;
}



UART_BUF buf_uart1;                       //串口1缓存

//import from main.c
extern wait_event_t command_event;	   //命令事件
void uart0_read_data(u8_t *buff)
{
	//获取到串口1的数据
    if(buf_uart1.index >= BUFLEN)
    {
        buf_uart1.index = 0 ;
    }
    else
    {
        buf_uart1.buf[buf_uart1.index++] = buff[0];
    }

    if(command_event != 0)                  //只要创建了信号量command_event，就不为0
        wake_up(command_event);             //传送信号量
}

void clear_buf_uart1(void)
{
    unsigned int i = 0 ,c ;
    c = BUFLEN +1 ;
    USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);

    for( i = 0 ; i < c ; i ++)
    {
        buf_uart1.buf[i] = 0x0 ;
    }

    buf_uart1.index = 0 ;

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}


void USART1_IRQHandler()
{
	__IO u32_t sr;
	uint8_t data;
	int need_sched = 0;

	enter_interrupt();
	
 	sr = USART1->SR;

        if( sr & (1L<<5))  //rx
	{
		data = USART1->DR;
                uart0_read_data(&data);
	}
        if( sr & (1L<<6))  //tx
	{
		if(u1_send.sending.val)
		{
			need_sched = uart1_send_next();
		}
		else
		{
			USART_ClearITPendingBit(USART1, USART_IT_TC);
		}
	} 
	
	exit_interrupt(need_sched);
}


void USART3_IRQHandler()
{
	__IO int i;
	__IO u32_t sr;
	int need_sched = 0;
	
	enter_interrupt();

	sr = USART3->SR;
	if( sr & (1L<<5))  //rxne
	{
		i = USART3->DR;
	}
	if( sr & (1L<<6))  //txe
	{
		if(u3_send.sending.val)
		{
			need_sched = uart3_send_next();
		}
		else
		{
			USART_ClearITPendingBit(USART3, USART_IT_TC);
		}
	} 
	
	exit_interrupt(need_sched);
}


