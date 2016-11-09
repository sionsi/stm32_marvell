#ifndef __ARCH_CC_H__
#define __ARCH_CC_H__


/* 包含相关头文件 */
#include "app_cfg.h"

/* 提供标准错误代号 */
#define LWIP_PROVIDE_ERRNO


/* 定义与编译器无关的数据类型 */


/* 定义指针类型 */
typedef u32_t mem_ptr_t;


/* 结构体 */
#define PACK_STRUCT_FIELD(x) __packed x 
#define PACK_STRUCT_STRUCT	
#define PACK_STRUCT_BEGIN 	 __packed
#define PACK_STRUCT_END


/* 平台调试输出和断言 */
#if 1

/* 这个串口格式化输出要自己实现 */
//extern void printf(char *fmt,...);
#include "stdio.h"


#define U8_F "c"
#define S8_F "c"
#define X8_F "x"
#define U16_F "u"
#define S16_F "d"
#define X16_F "x"
#define U32_F "u"
#define S32_F "d"
#define X32_F "x"

#define LWIP_DEBUG


//#define ETHARP_DEBUG                    LWIP_DBG_ON     
#define NETIF_DEBUG                     LWIP_DBG_ON     
//#define PBUF_DEBUG                      LWIP_DBG_ON
//#define API_LIB_DEBUG                   LWIP_DBG_ON
//#define API_MSG_DEBUG                   LWIP_DBG_ON
//#define SOCKETS_DEBUG                   LWIP_DBG_ON
//#define ICMP_DEBUG                      LWIP_DBG_ON
//#define IGMP_DEBUG                      LWIP_DBG_ON
//#define INET_DEBUG                      LWIP_DBG_ON
#define IP_DEBUG                        LWIP_DBG_ON     
//#define IP_REASS_DEBUG                  LWIP_DBG_ON
//#define RAW_DEBUG                       LWIP_DBG_ON
//#define MEM_DEBUG                       LWIP_DBG_ON
//#define MEMP_DEBUG                      LWIP_DBG_ON
//#define SYS_DEBUG                       LWIP_DBG_ON
#define TCP_DEBUG                       LWIP_DBG_ON
//#define TCP_INPUT_DEBUG                 LWIP_DBG_ON
//#define TCP_FR_DEBUG                    LWIP_DBG_ON
//#define TCP_RTO_DEBUG                   LWIP_DBG_ON
//#define TCP_CWND_DEBUG                  LWIP_DBG_ON
//#define TCP_WND_DEBUG                   LWIP_DBG_ON
#define TCP_OUTPUT_DEBUG                LWIP_DBG_ON
//#define TCP_RST_DEBUG                   LWIP_DBG_ON
//#define TCP_QLEN_DEBUG                  LWIP_DBG_ON
//#define UDP_DEBUG                       LWIP_DBG_ON     
#define TCPIP_DEBUG                     LWIP_DBG_ON
//#define PPP_DEBUG                       LWIP_DBG_ON
//#define SLIP_DEBUG                      LWIP_DBG_ON
//#define DHCP_DEBUG                      LWIP_DBG_ON     
//#define AUTOIP_DEBUG                    LWIP_DBG_ON
//#define SNMP_MSG_DEBUG                  LWIP_DBG_ON
//#define SNMP_MIB_DEBUG                  LWIP_DBG_ON
//#define DNS_DEBUG                       LWIP_DBG_ON


#define lwip_printf printf

#define LWIP_PLATFORM_DIAG(x)	 { lwip_printf x; }
#define LWIP_PLATFORM_ASSERT(x)  { lwip_printf("Assertion \"%s\" failed at line %d in %s\n", \
                                     x, __LINE__, __FILE__); while(1); }

#define LWIP_PLATFORM_ERR(x)  { lwip_printf("Assertion \"%s\" failed at line %d in %s\n", \
                                     x, __LINE__, __FILE__);}
#else

/* 如果你不能实现上面的串口格式化输出, 那么 #if 后应改为 0, 以下的将被编译 */

#define LWIP_PLATFORM_DIAG(x)
#define LWIP_PLATFORM_ASSERT(x)  { while(1); }

#endif


/* 临界区的保护( 使用uCOS-II的第三种临界处理方式 ) */
#define SYS_ARCH_DECL_PROTECT(x) u32_t cpu_sr
#define SYS_ARCH_PROTECT(x)      cpu_sr = local_irq_save()
#define SYS_ARCH_UNPROTECT(x)    local_irq_restore(cpu_sr)


#endif /* __ARCH_CC_H__ */


