#define DEBUG
#include "drivers.h"
#include "app.h"
#include "api.h"

#include "cfg80211.h"
#include "defs.h"
#include "type.h"
#include "types.h"

#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "wpa.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "webserver.h"
#include "usr_cfg.h"


#if SUPPORT_WEBSERVER

const char err_404_page[] = "HTTP/1.1 404 Object Not Found\r\n\
Server: CSIC-WIFIBOARD-WebServer-v1.0\r\n\
Connection: close\r\n\
Content-Length: 75\r\n\
Content-Type: text/html\r\n\
\r\n\
<html><head><title>Error</title></head><body>Object Not Found</body></html>";

const char page_head[] = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\r\n\
<html xmlns=\"http://www.w3.org/1999/xhtml\">\r\n\
<head>\r\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\" />\r\n\
<title></title>\r\n\
<style type=\"text/css\">\r\n\
<!--\r\n\
.STYLE16 {font-size: 16px}\r\n\
.STYLE18 {font-size: 24px; font-weight: bold; }\r\n\
-->\r\n\
</style>\r\n\
</head>\r\n\r\n";


const char http_body_head[] = "<body>\r\n\
<form id=\"form1\" name=\"form1\" method=\"post\" action=\"\">\r\n\
<center> \r\n\
<table width=\"500\" height=\"300\" border=\"0\" bordercolor=\"#000000\" bgcolor=\"#FFFFFF\">\r\n";

const char http_body_tail[] = "</table>\r\n\
</center>\r\n\
</form>\r\n\
</body>\r\n\
</html>";



const char str_content_len[]  = "Content-Length";
const char str_get[]  = "GET";
const char str_post[]  = "POST";
const char str_line_end[]  = "\r\n";
const char str_sect_end[]  = "\r\n\r\n";

const char str_index_page[]  = "index.html";
const char str_device_info_page[]  = "device_info.html";
const char str_wifi_link_page[] = "wifi_link.html";
const char str_ip_addr_page[] = "ip_addr.html";
const char str_tcp_server_page[] = "tcp_server.html";
const char str_tcp_client_page[] = "tcp_client.html";	
const char str_remote_ctl_page[] = "remote_ctl.html";	
const char str_reset_page[] = "reset.html";
const char str_uart_page[] = "uart.html";
const char str_io_page[] = "io_ctrl.html";
const char str_update_page[] = "update.html";
const char str_cust_page[] = "cust_page.html";
const char str_login_page[] = "password.html";
const char str_logo_gif[]  = "logo.gif";


char web_buff[WEB_BUFF_SIZE];
extern mutex_t socket_mutex;
WEB_CFG web_cfg;

extern void send_post_status(int s, int _step);
extern void index_page_send(UCHAR s, CHAR _step);
extern void logo_gif_send(UCHAR s, CHAR _step);
extern int web_body_add_option_str(char *buff, const char *tag, const char *str);
extern int web_body_add_input_str(char *buff, const char *tag, const char *str);
extern int mem_move(void *start, void *end, int mv_len, int direct);
extern int get_post_tag_value(const char *tag, const char *buff, char *str_val);
extern char *get_body_tag(const char *tag, const char *buff, int *len);
extern void str_replace(char *buff, char *old, char *new);

extern int write_update_data(uint8_t *buff, int size);
extern int erase_update_erea(uint32_t addr, uint32_t size);
extern void write_update_data_finish(void);

int web_data_send(UCHAR s, const char *data, int len)
{
	int remain = len;
	char *p_data = (char*)data;
	int ret;
	web_cfg.web_sm = WEB_SM_SEND_DATA;
	if(s == 0xff)
		return len;
again:
	mutex_lock(socket_mutex);
	ret = send(s, p_data, remain, 0);
	mutex_unlock(socket_mutex);
	if(ret < 0){
		web_cfg.web_sm = WEB_SM_IDLE;
		p_err("e:%d", ret);
	}else{
		data += ret;
		remain -= ret;
		if(remain > 0){
			p_dbg("remain:%d", remain);
			goto again;
		}
	}
	return ret;
}

//标题
const char local_text1[] = "<tr>\r\n\
    <td  height=\"70\" colspan=\"3\"><div align=\"center\"><span class=\"STYLE18\">";

const char local_text2[] = "</span></div><hr /></td>\r\n\
  	</tr>\r\n";

void make_sub_page_title(char *name)
{
	sprintf(web_buff, "%s%s%s", local_text1, name, local_text2);
}


//选择 AP 还是STA
const char local_text3[] = "<tr>\r\n\
<td  height=\"30\" colspan=\"3\"> <span class=\"STYLE16\">\r\n\
<label></label>\r\n\
</span><span class=\"STYLE16\">\r\n\
<label>        </label>\r\n\
</span>\r\n\
<label>\r\n\
<div align=\"center\"><span class=\"STYLE16\">\r\n\
<input name=\"link_mode\" type=\"radio\" value=\"1\" />\r\n\
AP  \r\n\
<input name=\"link_mode\" type=\"radio\" value=\"0\" />\r\n\
STATION\r\n\
</span></div>\r\n\
</label>\r\n\
<span class=\"STYLE16\"></span></td>\r\n\
</tr>\r\n";

//网络设置名称
const char local_text4[]  = "<tr>\r\n\
<td width=\"38%\" height=\"30\"><div align=\"right\"><span class=\"STYLE16\">名称 :</span></div></td>\r\n\
<td width=\"38%\"><div align=\"left\"><span class=\"STYLE16\">\r\n\
<input name=\"link_ssid\" type=\"text\" id=\"link_ssid\" maxlength=\"32\"/>\r\n\
</span></div></td>\r\n\
<td width=\"24%\">&nbsp;</td>\r\n\
</tr>\r\n";

//网络设置密码
const char local_text5[]  = "<tr>\r\n\
<td height=\"30\"><div align=\"right\"><span class=\"STYLE16\">密码 :</span></div></td>\r\n\
<td><div align=\"left\"><span class=\"STYLE16\">\r\n\
<input name=\"link_key\" type=\"text\" id=\"link_key\" maxlength=\"13\"/>\r\n\
</span></div></td>\r\n\
<td>&nbsp;</td>\r\n\
</tr>\r\n";

//控制LED 开
const char local_text6[]  ="<tr>\r\n\
<td colspan=\"2\"><div align=\"center\"><span class=\"STYLE17\"><span class=\"STYLE16\">\r\n\
<input name=\"io_ctrl\" type=\"submit\" id=\"io_ctrl\" value=\"OPEN LED1\" />\r\n\
</span></span></div></td>\r\n\
</tr>\r\n";

//控制LED 关
const char local_text7[]  ="<tr>\r\n\
<td colspan=\"2\"><div align=\"center\"><span class=\"STYLE17\"><span class=\"STYLE16\">\r\n\
<input name=\"io_ctrl\" type=\"submit\" id=\"io_ctrl\" value=\"CLOSE LED1\" />\r\n\
</span></span></div></td>\r\n\
</tr>\r\n";

const char local_text8[]  ="<tr>\r\n\
<td colspan=\"2\"><div align=\"center\"><span class=\"STYLE17\"><span class=\"STYLE16\">\r\n\
<input name=\"bt_save\" type=\"submit\" id=\"bt_save\" value=\"确定\" />\r\n\
</span></span></div></td>\r\n\
</tr>\r\n";




const char local_text_linkmode_ap[]  = "input name=\"link_mode\" type=\"radio\" value=\"1\"";
const char local_text_linkmode_sta[]  = "input name=\"link_mode\" type=\"radio\" value=\"0\"";
const char local_text_checked[]  = " checked=\"checked\" ";

const char local_text_linkssid[]  = "<input name=\"link_ssid\" type=\"text\" id=\"link_ssid\" maxlength=\"32\"/>";
const char local_text_linkkey[]  = "<input name=\"link_key\" type=\"text\" id=\"link_key\" maxlength=\"13\"/>";



int wifi_link_page_assemble(UCHAR s, CHAR _sub_step)
{
	static uint8_t step = 0;
	char tmp[64];
	int ret = 0;
	if(_sub_step != -1)
		step = _sub_step;

	switch(step)
	{
		case 0:
			step = 1;
			ret = web_data_send(s, page_head, SIZEOF_STR(page_head));
			return ret;
		case 1:
			step = 2;
			ret = web_data_send(s, http_body_head, SIZEOF_STR(http_body_head));
			return ret;
		case 2:
			step = 3;
			make_sub_page_title("网络设置");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 3:
			step = 4;
			memcpy(web_buff, local_text3, SIZEOF_STR(local_text3));
			web_buff[SIZEOF_STR(local_text3)] = 0;
			if(web_cfg.link.mode)
			{
				web_body_add_input_str(web_buff, 
				local_text_linkmode_ap, 
				local_text_checked);
			}else{
				web_body_add_input_str(web_buff, 
				local_text_linkmode_sta, 
				local_text_checked);
			}
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 4:
		{
			step = 5;
			memcpy(web_buff, local_text4, SIZEOF_STR(local_text4));
			web_buff[SIZEOF_STR(local_text4)] = 0;

			sprintf(tmp, " value=\"%s\" ", web_cfg.link.essid);

			web_body_add_input_str(web_buff, 
				local_text_linkssid, 
				tmp); 
  			ret = web_data_send(s, web_buff, strlen(web_buff));
		}
			return ret;
		case 5:
			step = 12;
			memcpy(web_buff, local_text5, SIZEOF_STR(local_text5));
			web_buff[SIZEOF_STR(local_text5)] = 0;

			sprintf(tmp, " value=\"%s\" ", web_cfg.link.key);

			web_body_add_input_str(web_buff, 
				local_text_linkkey, 
				tmp); 
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 6:
			step = 7;
			return ret;
		case 7:
			step = 8;
			return ret;
		case 8:
			step = 9;
			return ret;
		case 9:
			step = 10;
			return ret;
		case 10:
			step = 11;
			return ret;
		case 11:
			step = 12;
			return ret;
		case 12:
			step = 13;
			assert(SIZEOF_STR(local_text8) < WEB_BUFF_MAX_VALID);
			memcpy(web_buff, local_text8, SIZEOF_STR(local_text8));
			web_buff[SIZEOF_STR(local_text8)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;	
		case 13:
			step = 14;
			ret = web_data_send(s, http_body_tail, SIZEOF_STR(http_body_tail));
			return ret;
		default:
			step = 0;
			return 0;
	}
}


const char ip_addr_text1[]  = "<tr>\r\n\
<td width=\"31%\" height=\"28\"><div align=\"right\"><span class=\"STYLE16\">IP 地址:</span></div></td>\r\n\
<td width=\"69%\"><div align=\"left\"><span class=\"STYLE16\">\r\n\
<input name=\"ip_ipaddr\" type=\"text\" id=\"ip_ipaddr\" maxlength=\"15\"/>\r\n\
</span></div></td>\r\n\
</tr>\r\n";

const char ip_addr_text2[]  = "<tr>\r\n\
<td width=\"31%\" height=\"28\"><div align=\"right\"><span class=\"STYLE16\">网关地址:</span></div></td>\r\n\
<td width=\"69%\"><div align=\"left\"><span class=\"STYLE16\">\r\n\
<input name=\"ip_gw\" type=\"text\" id=\"ip_gw\" maxlength=\"15\"/>\r\n\
</span></div></td>\r\n\
</tr>\r\n";

const char ip_addr_text3[]  = "<tr>\r\n\
<td width=\"31%\" height=\"28\"><div align=\"right\"><span class=\"STYLE16\">子网掩码:</span></div></td>\r\n\
<td width=\"69%\"><div align=\"left\"><span class=\"STYLE16\">\r\n\
<input name=\"ip_mask\" type=\"text\" id=\"ip_mask\" maxlength=\"15\"/>\r\n\
</span></div></td>\r\n\
</tr>\r\n";

const char ip_addr_text4[]  = "<tr>\r\n\
<td width=\"31%\" height=\"28\"><div align=\"right\"><span class=\"STYLE16\">DNS地址:</span></div></td>\r\n\
<td width=\"69%\"><div align=\"left\"><span class=\"STYLE16\">\r\n\
<input name=\"ip_dns\" type=\"text\" id=\"ip_dns\" maxlength=\"15\"/>\r\n\
</span></div></td>\r\n\
</tr>\r\n";


const char ip_text_ipaddr[]  = "<input name=\"ip_ipaddr\" type=\"text\" id=\"ip_ipaddr\" maxlength=\"15\"/>";
const char ip_text_mask[]  = "<input name=\"ip_mask\" type=\"text\" id=\"ip_mask\" maxlength=\"15\"/>";
const char ip_text_gw[]  = "<input name=\"ip_gw\" type=\"text\" id=\"ip_gw\" maxlength=\"15\"/>";
const char ip_text_dns[]  = "<input name=\"ip_dns\" type=\"text\" id=\"ip_dns\" maxlength=\"15\"/>";
const char ip_text_auto[]  = "<input name=\"ip_auto\" type=\"checkbox\" id=\"ip_auto\" value=\"1\" />";

int ip_addr_page_assemble(UCHAR s, CHAR _sub_step)
{
	static uint8_t step = 0;
	char tmp[64];
	uint32_t dns_server;
	//unsigned char ip_addr[4];
	int ret =  0;
	if(_sub_step != -1)
		step = _sub_step;

	

	switch(step)
	{
		case 0:
			step = 1;
			ret = web_data_send(s, page_head, SIZEOF_STR(page_head));
			return ret;
		case 1:
			step = 2;
			ret = web_data_send(s, http_body_head, SIZEOF_STR(http_body_head));
			return ret;
		case 2:
			step = 3;
			make_sub_page_title("主机状态");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 3:
			step = 4;
			memcpy(web_buff, ip_addr_text1, SIZEOF_STR(ip_addr_text1));
			web_buff[SIZEOF_STR(ip_addr_text1)] = 0;
			sprintf(tmp, " value=\"%u.%u.%u.%u\" ", ip4_addr1(&p_netif->ip_addr.addr),  
			ip4_addr2(&p_netif->ip_addr.addr), 
			ip4_addr3(&p_netif->ip_addr.addr), 
			ip4_addr4(&p_netif->ip_addr.addr));
			web_body_add_input_str(web_buff, 
				ip_text_ipaddr, 
				tmp);
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 4:
			step = 5;
			memcpy(web_buff, ip_addr_text2, SIZEOF_STR(ip_addr_text2));
			web_buff[SIZEOF_STR(ip_addr_text2)] = 0;
			sprintf(tmp, " value=\"%u.%u.%u.%u\" ", ip4_addr1(&p_netif->gw.addr),
			ip4_addr2(&p_netif->gw.addr),
			ip4_addr3(&p_netif->gw.addr),
			ip4_addr4(&p_netif->gw.addr));
			web_body_add_input_str(web_buff, 
				ip_text_gw, 
				tmp);
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 5:
			step = 6;
			memcpy(web_buff, ip_addr_text3, SIZEOF_STR(ip_addr_text3));
			web_buff[SIZEOF_STR(ip_addr_text3)] = 0;
			sprintf(tmp, " value=\"%u.%u.%u.%u\" ", ip4_addr1(&p_netif->netmask.addr),
			ip4_addr2(&p_netif->netmask.addr),
			ip4_addr3(&p_netif->netmask.addr),
			ip4_addr4(&p_netif->netmask.addr));
			web_body_add_input_str(web_buff, 
				ip_text_mask, 
				tmp);
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 6:
			step = 7;
			dns_server = dns_getserver(0).addr;
			memcpy(web_buff, ip_addr_text4, SIZEOF_STR(ip_addr_text4));
			web_buff[SIZEOF_STR(ip_addr_text4)] = 0;
			sprintf(tmp, " value=\"%u.%u.%u.%u\" ", ip4_addr1(&dns_server),
				ip4_addr2(&dns_server),
				ip4_addr3(&dns_server),
				ip4_addr4(&dns_server));
			web_body_add_input_str(web_buff, 
				ip_text_dns, 
				tmp);
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 7:
			step = 8;
			return ret;
		case 8:
			step = 9;
			return ret;	
		case 9:
			step = 10;
			ret = web_data_send(s, http_body_tail, SIZEOF_STR(http_body_tail));
			return ret;
		default:
			step = 0;
			return 0;
	}
}



const char reset_text1[]  = "<tr>\r\n\
<td height=\"42\"><div align=\"center\"><span class=\"STYLE16\">\r\n\
<input name=\"factory_reset\" type=\"checkbox\" id=\"factory_reset\" value=\"1\" />\r\n\
是否要恢复出厂设置</span></div>\r\n\
<span class=\"STYLE16\">\r\n\
<label></label>\r\n\
</span></td>\r\n\
</tr>\r\n";


int reset_page_assemble(UCHAR s, CHAR _sub_step)
{
	static uint8_t step = 0;
	//unsigned char ip_addr[4];
	int ret;
	if(_sub_step != -1)
		step = _sub_step;

	switch(step)
	{
		case 0:
			step = 1;
			ret = web_data_send(s, page_head, SIZEOF_STR(page_head));
			return ret;
		case 1:
			step = 2;
			ret = web_data_send(s, http_body_head, SIZEOF_STR(http_body_head));
			return ret;
		case 2:
			step = 3;
			make_sub_page_title("重启板子");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 3:
			step = 5;
			//assert(SIZEOF_STR(reset_text1) < WEB_BUFF_MAX_VALID);
			memcpy(web_buff, reset_text1, SIZEOF_STR(reset_text1));
			web_buff[SIZEOF_STR(reset_text1)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 5:
			step = 6;
			memcpy(web_buff, local_text8, SIZEOF_STR(local_text8));
			web_buff[SIZEOF_STR(local_text8)] = 0;
			str_replace(web_buff, "确定", "重启");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;	
		case 6:
			step = 7;
			ret = web_data_send(s, http_body_tail, SIZEOF_STR(http_body_tail));
			return ret;
		default:
			step = 0;
			return 0;
	}
}


//DHT11 温湿度采集
const char dht11_text1[]  = "<tr>\r\n\
<td width=\"33%\" height=\"43\"><div align=\"right\"><span class=\"STYLE16\">传感器数据:</span></div></td>\r\n\
<td width=\"67%\"><span class=\"STYLE16\">\r\n\
<div align=\"left\">\r\n\
<input name=\"dht11_addr\" type=\"text\" id=\"dht11_addr\" maxlength=\"80\"/>\r\n\
</div>\r\n\
</span></td>\r\n\
</tr>\r\n";

const char DHT11_text[]  = "<input name=\"dht11_addr\" type=\"text\" id=\"dht11_addr\" maxlength=\"80\"/>";
extern uint8_t Data_Array[5];				//温湿度数组

int login_page_assemble(UCHAR s, CHAR _sub_step)
{
	static uint8_t step = 0;
	char tmp[64];
	//unsigned char ip_addr[4];
	int ret=0;
	if(_sub_step != -1)
		step = _sub_step;

	switch(step)
	{
		case 0:
			step = 1;
			ret = web_data_send(s, page_head, SIZEOF_STR(page_head));
			return ret;
		case 1:
			step = 2;
			ret = web_data_send(s, http_body_head, SIZEOF_STR(http_body_head));
			return ret;
		case 2:
			step = 3;
			make_sub_page_title("数据采集");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 3:
			step = 5;
		   	assert(SIZEOF_STR(dht11_text1) < WEB_BUFF_MAX_VALID);
			memcpy(web_buff, dht11_text1, SIZEOF_STR(dht11_text1));
			web_buff[SIZEOF_STR(dht11_text1)] = 0;
			//printf("传感器湿度=%d.%d%%RH 温度=%d.%d℃ 校验和=%d\r\n",Data_Array[0],Data_Array[1],Data_Array[2],Data_Array[3],Data_Array[4]);
			sprintf(tmp, " value=\"湿度:%d.%d%%RH 温度:%d.%d℃ \" ", Data_Array[0],Data_Array[1],Data_Array[2],Data_Array[3]);
			web_body_add_input_str(web_buff, DHT11_text, tmp);
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 5:
			step = 6;
			return ret;	
		case 6:
			step = 7;
			ret = web_data_send(s, http_body_tail, SIZEOF_STR(http_body_tail));
			return ret;
		default:
			step = 0;
			return 0;
	}
}



int io_page_assemble(UCHAR s, CHAR _sub_step)
{
	static uint8_t step = 0;
	int ret = 0;
	if(_sub_step != -1)
		step = _sub_step;

	switch(step)
	{
		case 0:
			step = 1;
			ret = web_data_send(s, page_head, SIZEOF_STR(page_head));
			return ret;
		case 1:
			step = 2;
			ret = web_data_send(s, http_body_head, SIZEOF_STR(http_body_head));
			return ret;
		case 2:
			step = 3;
			make_sub_page_title("控制板子");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 3:
			step = 4;
			memcpy(web_buff, local_text6, SIZEOF_STR(local_text6));
			web_buff[SIZEOF_STR(local_text6)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 4:
			step = 5;
			memcpy(web_buff, local_text7, SIZEOF_STR(local_text7));
			web_buff[SIZEOF_STR(local_text7)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 5:
			step = 6;
			memcpy(web_buff, local_text6, SIZEOF_STR(local_text6));
			web_buff[SIZEOF_STR(local_text6)] = 0;
			
			str_replace(web_buff, "OPEN LED1", "OPEN LED2");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 6:
			step = 7;
			memcpy(web_buff, local_text7, SIZEOF_STR(local_text7));
			web_buff[SIZEOF_STR(local_text7)] = 0;
			
			str_replace(web_buff, "CLOSE LED1", "CLOSE LED2");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 7:
			step = 8;
			memcpy(web_buff, local_text6, SIZEOF_STR(local_text6));
			web_buff[SIZEOF_STR(local_text6)] = 0;
			
			str_replace(web_buff, "OPEN LED1", "OPEN LED3");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 8:
			step = 9;
			memcpy(web_buff, local_text7, SIZEOF_STR(local_text7));
			web_buff[SIZEOF_STR(local_text7)] = 0;
			
			str_replace(web_buff, "CLOSE LED1", "CLOSE LED3");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 9:
			step = 10;
			return ret;	
		case 10:
			step = 11;
			ret = web_data_send(s, http_body_tail, SIZEOF_STR(http_body_tail));
			return ret;
		default:
			step = 0;
			return 0;
	}
}




//服务器的地址
const char client_text1[]  = "<tr>\r\n\
<td width=\"33%\" height=\"43\"><div align=\"right\"><span class=\"STYLE16\">服务器地址:</span></div></td>\r\n\
<td width=\"67%\"><span class=\"STYLE16\">\r\n\
<div align=\"left\">\r\n\
<input name=\"server_addr\" type=\"text\" id=\"server_addr\" maxlength=\"64\"/>\r\n\
</div>\r\n\
</span></td>\r\n\
</tr>\r\n";

const char server_addr_text[]  = "<input name=\"server_addr\" type=\"text\" id=\"server_addr\" maxlength=\"64\"/>";

//TCPSERVER TCPCLIENT UDPSERVER UDPCLIENT
const char server_choose1[] = "<tr>\r\n\
<td  height=\"30\" colspan=\"3\"> <span class=\"STYLE16\">\r\n\
<label></label>\r\n\
</span><span class=\"STYLE16\">\r\n\
<label>板子是服务器</label>\r\n\
</span>\r\n\
<label>\r\n\
<div align=\"center\"><span class=\"STYLE16\">\r\n\
<input name=\"socket_mode\" type=\"radio\" value=\"1001\" checked=\"checked\" />\r\n\
TCP服务器 端口:1001\r\n\
<input name=\"socket_mode\" type=\"radio\" value=\"1003\" />\r\n\
UDP服务器 端口:1003\r\n\
</span></div>\r\n\
</label>\r\n\
<span class=\"STYLE16\"></span></td>\r\n\
</tr>\r\n";

const char server_choose2[] = "<tr>\r\n\
<td  height=\"30\" colspan=\"3\"> <span class=\"STYLE16\">\r\n\
<label></label>\r\n\
</span><span class=\"STYLE16\">\r\n\
<label>板子是客户端</label>\r\n\
</span>\r\n\
<label>\r\n\
<div align=\"center\"><span class=\"STYLE16\">\r\n\
<input name=\"socket_mode\" type=\"radio\" value=\"1000\" />\r\n\
TCP客户端 端口:1000\r\n\
<input name=\"socket_mode\" type=\"radio\" value=\"1002\" />\r\n\
UDP 客户端 端口:1002\r\n\
</span></div>\r\n\
</label>\r\n\
<span class=\"STYLE16\"></span></td>\r\n\
</tr>\r\n";

int tcp_client_page_assemble(UCHAR s, CHAR _sub_step)
{
	static uint8_t step = 0;
	char tmp[64 + 1];
	//unsigned char ip_addr[4];
	int ret = 0;
	if(_sub_step != -1)
		step = _sub_step;

	

	switch(step)
	{
		case 0:
			step = 1;
			ret = web_data_send(s, page_head, SIZEOF_STR(page_head));
			return ret;
		case 1:
			step = 2;
			ret = web_data_send(s, http_body_head, SIZEOF_STR(http_body_head));
			return ret;
		case 2:
			step = 3;
			make_sub_page_title("SOCKET TCP UDP 配置");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 3:
			step = 4;
			memcpy(web_buff, server_choose1, SIZEOF_STR(server_choose1));
			web_buff[SIZEOF_STR(server_choose1)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;	
		case 4:
			step = 5; 
			memcpy(web_buff, server_choose2, SIZEOF_STR(server_choose2));
			web_buff[SIZEOF_STR(server_choose2)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 5:
			step = 6;
			assert(SIZEOF_STR(client_text1) < WEB_BUFF_MAX_VALID);
			memcpy(web_buff, client_text1, SIZEOF_STR(client_text1));
			web_buff[SIZEOF_STR(client_text1)] = 0;

			sprintf(tmp, " value=\"%u.%u.%u.%u\" ", 192,168 ,1 ,100);
			web_body_add_input_str(web_buff, server_addr_text, tmp);
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;	
		case 6:
			step = 7;
			memcpy(web_buff, local_text8, SIZEOF_STR(local_text8));
			web_buff[SIZEOF_STR(local_text8)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;	
		case 7:
			step = 8;
			ret = web_data_send(s, http_body_tail, SIZEOF_STR(http_body_tail));
			return ret;
		default:
			step = 0;
			return 0;
	}
}


extern void show_tcpip_info(void);
int handle_wifilink_post(int s, char *buff)
{
	int i;
	char ssid[32 + 1];
	char key[32 + 1];
	int auth_mode;

	i = get_post_tag_value("link_mode", buff, 0);
	web_cfg.link.mode = (i == 1)?1:0;

	i = get_post_tag_value("link_channel", buff, 0);
	if(i > 0 && i < 14)
		i = i;
	else
		i = 1;
	web_cfg.link.channel = i;

	i = get_post_tag_value("link_ssid", buff, ssid);
	if(i == 0)
	{
		if(strlen(ssid) < 32)
			strcpy(web_cfg.link.essid, ssid);
	}
	i = get_post_tag_value("link_key", buff, key);
	if(i == 0)
	{
		if(strlen(key) < 32)
			strcpy(web_cfg.link.key, key);
	}else{
		memset(web_cfg.link.key, 0, 14);
	}
	
	i = get_post_tag_value("link_security", buff, 0);
	auth_mode = i;
	
	send_post_status(s, 0);

	if(web_cfg.link.mode == 0)	//sta
	{

                p_dbg("创思通信STM32 WIFI连接到:%s, 密码:%s", web_cfg.link.essid, web_cfg.link.key);
		wifi_set_mode(MODE_STATION);
		
		wifi_connect(web_cfg.link.essid, web_cfg.link.key);
		
		if (is_wifi_connected())
		{
			int i, wait_time = 10;

			auto_get_ip();

			for (i = 0; i < wait_time; i++)
			{
				if (p_netif->ip_addr.addr)
					break;
				sleep(1000);
			}

			if (p_netif->ip_addr.addr)
			{
				show_tcpip_info();
			}
		}

		
	}else{

		wifi_set_mode(MODE_AP);

		wifi_ap_cfg(web_cfg.link.essid, web_cfg.link.key, auth_mode, web_cfg.link.channel, 4);

	}

	return 0;
	
}


int handle_ip_addr_post(int s, char *buff)
{
	int i;
	char tmp[64];

	i = get_post_tag_value("ip_ipaddr", buff, tmp);
	if(i == 0){
		web_cfg.ip.ip = inet_addr(tmp);
	}

	i = get_post_tag_value("ip_gw", buff, tmp);
	if(i == 0){
		web_cfg.ip.gw = inet_addr(tmp);
	}

	i = get_post_tag_value("ip_mask", buff, tmp);
	if(i == 0){
		web_cfg.ip.msk = inet_addr(tmp);
	}

	i = get_post_tag_value("ip_dns", buff, tmp);
	if(i == 0){
		web_cfg.ip.dns = inet_addr(tmp);
	}

	i = get_post_tag_value("ip_auto", buff, 0);
	if(i != 1)
		web_cfg.ip.auto_get = 0;
	else
		web_cfg.ip.auto_get = 1;
	
	send_post_status(s, 0);

	//todo
	return 0;
	
}

int handle_login_post(int s, char *buff)
{
	int i;
	char tmp[64];

	i = get_post_tag_value("log_password", buff, tmp);
	if(i == 0)
	{
		if(strlen(tmp) <= 16)
		{
			strcpy(web_cfg.password, tmp);
		}
	}else{
		memset(web_cfg.password, 0, 32);
	}
	
	send_post_status(s, 0);

	//todo
	return 0;
}

int handle_reset_post(int s, char *buff)
{
	int i;

	i = get_post_tag_value("factory_reset", buff, 0);
	if(i == 1){
		p_dbg("Factory Mode");
	}
	send_post_status(s, 0);

	soft_reset();

	return 0;
	
}



int handle_io_post(int s, char *buff)
{
	char *pstr = NULL;
        p_dbg(buff);

	pstr = strstr(buff,"io_ctrl=OPEN");
	if(pstr)
	{
		if(strstr(pstr,"LED1"))
		{
                        LED1(0);
		}
		else if(strstr(pstr,"LED2"))
		{
			LED2(0);
		}
		else if (strstr(pstr,"LED3"))
		{
                        LED3(0);
                        RELAY(0);
		}
		return 0;
	}

	pstr = NULL;
	pstr = strstr(buff,"io_ctrl=CLOSE");
	if(pstr)
	{
		if(strstr(pstr,"LED1"))
		{
                        LED1(1);
		}
		else if(strstr(pstr,"LED2"))
		{
			LED2(1);
		}
		else if (strstr(pstr,"LED3"))
		{
                        LED3(1);
                        RELAY(1);
		}
		return 0;
	}

	return 0;
}




int handle_tcp_client_post(int s, char *buff)
{
	char tmp[64 + 1];
	int i;
	i = get_post_tag_value("server_addr", buff, tmp);
	printf("i=%d",i);
	p_dbg(tmp);
	if(i == 0)
	{
		web_cfg.dtu.server_ip = 1;
		strcpy(web_cfg.dtu.server_name, tmp);
	}

	i = get_post_tag_value("socket_mode", buff, tmp);
	printf("i=%d",i);
	p_dbg(tmp);

	if(i == 0)
	{
		choose_socket(tmp);
	}

	send_post_status(s, 0);
	return 0;
	
}


/*
 *发送一个具体的页面，页面内容可先在Dreamweaver编辑，
 *然后放在这里分段发送出去，
 *上面assemble相关的函数使用相同的处理方式
 *
*/
int devcie_info_page_assemble(UCHAR s, CHAR _sub_step)
{
	static uint8_t step = 0;
	int ret;
	if(_sub_step != -1)
		step = _sub_step;

	switch(step)
	{
		case 0:
			step = 1;
			ret = web_data_send(s, page_head, SIZEOF_STR(page_head) - 1);
			return ret;
		case 1:
			step = 2;
			ret = web_data_send(s, http_body_head, SIZEOF_STR(http_body_head) - 1);
			return ret;
		case 2:
			step = 3;
			sprintf(web_buff, "<tr bordercolor=\"#F0F0F0\">\r\n\
    			<td height=\"94\" colspan=\"2\"><div align=\"center\"><span class=\"STYLE18\"><p>创思通信引领物联时代</p></span></div>\r\n\
				<p>电话:18144070918</p>\r\n\
				<p>QQ:2357481431</p>\r\n\
				<p>更多产品请关注:csic.taobao.com</p>\r\n\
      			<hr /></td>\r\n\
  				</tr>\r\n");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 3:
			step = 4;
			ret = web_data_send(s, http_body_tail, SIZEOF_STR(http_body_tail));
			return ret;
		default:
			step = 0;
			return 0;
	}
}

/*
 *处理get请求
 *
 *
*/
void handle_web_get(UCHAR s)
{
	if(web_cfg.get_name[0] == 0 || strcmp(web_cfg.get_name, str_index_page) == 0){
		index_page_send(s, 0);
	}else if(strcmp(web_cfg.get_name, str_device_info_page) == 0){
		web_cfg.assemble_routing = devcie_info_page_assemble;
		sub_page_send(s, 0);
	}else if(strcmp(web_cfg.get_name, str_wifi_link_page) == 0){
		web_cfg.assemble_routing = wifi_link_page_assemble;
		sub_page_send(s, 0);
	}else if(strcmp(web_cfg.get_name, str_ip_addr_page) == 0){
		web_cfg.assemble_routing = ip_addr_page_assemble;
		sub_page_send(s, 0);
	}else if(strcmp(web_cfg.get_name, str_tcp_server_page) == 0){

	}else if(strcmp(web_cfg.get_name, str_tcp_client_page) == 0){
		web_cfg.assemble_routing = tcp_client_page_assemble;
		sub_page_send(s, 0);
	}else if(strcmp(web_cfg.get_name, str_remote_ctl_page) == 0){

	}else if(strcmp(web_cfg.get_name, str_reset_page) == 0){
		web_cfg.assemble_routing = reset_page_assemble;
		sub_page_send(s, 0);
	}else if(strcmp(web_cfg.get_name, str_uart_page) == 0){

	}else if(strcmp(web_cfg.get_name, str_io_page) == 0){
		web_cfg.assemble_routing = io_page_assemble;
		sub_page_send(s, 0);
	}else if(strcmp(web_cfg.get_name, str_update_page) == 0){

	}else if(strcmp(web_cfg.get_name, str_cust_page) == 0){

	}else if(strcmp(web_cfg.get_name, str_login_page) == 0){
		web_cfg.assemble_routing = login_page_assemble;
		sub_page_send(s, 0);
	}else if(strcmp(web_cfg.get_name, str_logo_gif) == 0)
		logo_gif_send(s, 0);
	else{
		p_err("unkown page:%s", web_cfg.get_name);
		web_data_send(s, err_404_page, SIZEOF_STR(err_404_page));
	}
}

/*
 *处理post请求
 *
 *
*/

void handle_web_post(int s)
{
	if(strcmp(web_cfg.get_name, str_wifi_link_page) == 0){
		handle_wifilink_post(s, web_buff);
	}else if(strcmp(web_cfg.get_name, str_ip_addr_page) == 0){
		handle_ip_addr_post(s, web_buff);
	}else if(strcmp(web_cfg.get_name, str_tcp_server_page) == 0){
	
	}else if(strcmp(web_cfg.get_name, str_tcp_client_page) == 0){
		handle_tcp_client_post(s, web_buff);
	}else if(strcmp(web_cfg.get_name, str_remote_ctl_page) == 0){

	}else if(strcmp(web_cfg.get_name, str_reset_page) == 0){
		handle_reset_post(s, web_buff);
	}else if(strcmp(web_cfg.get_name, str_uart_page) == 0){

	}else if(strcmp(web_cfg.get_name, str_io_page) == 0){
		handle_io_post(s, web_buff);
	}else if(strcmp(web_cfg.get_name, str_update_page) == 0){

	}else if(strcmp(web_cfg.get_name, str_cust_page) == 0){

	}else if(strcmp(web_cfg.get_name, str_login_page) == 0){
		handle_login_post(s, web_buff);
	}else
		p_err("unkown page:%s", web_cfg.get_name);
}

int is_str_end(char c)
{
	if(c == '\r' || c == '\n')
		return 1;
	else
		return 0;
}

void web_update_timer(void)
{
	if(web_cfg.is_update_post && (web_cfg.web_sm == WEB_SM_POST_GETMORE1))
	{
		if(web_cfg.update_data_timer++ > 16)
		{
			p_err("update timeout");
			web_cfg.is_update_post = 0;
			web_cfg.update_data_cnt = 0;
			web_cfg.web_sm =  WEB_SM_IDLE;
			//erase_update_erea(UPDATE_DATA_BASE_ADDR, );
		}
	}
}

void web_abort(void)
{
	web_cfg.web_sm = WEB_SM_IDLE;
	//if(web_cfg.need_reboot)
	//	Sys_reboot();
}

int handle_web_sm(UCHAR s, PUCHAR packet ,uint32_t packetlength)
{	
	int i;
	char *p_str;
#ifdef DEBUG
        /*if(packet){
                packet[packetlength] = 0;
                p_dbg("pkg:%s", packet);
        }  */
#endif
again:
	switch(web_cfg.web_sm)
	{
		case WEB_SM_IDLE: //idle状态下接受客户端的get和post请求
			if(strstr((const char*)packet, str_get)){
				p_dbg(str_get);
				web_cfg.web_sm = WEB_SM_GET_GETMORE;
				p_str = get_name((char*)str_get, (char*)packet, &i);
				if(p_str && i < GET_NAME_LEN)
				{
					memcpy(web_cfg.get_name, p_str, i);
					web_cfg.get_name[i] = 0;
				}else
					web_cfg.get_name[0] = 0;
			}else if(strstr((const char*)packet, str_post)){
				p_dbg(str_post);
				web_cfg.web_sm = WEB_SM_POST_GETMORE;
				web_cfg.post_len = 0;
				web_cfg.is_update_post = 0;
				p_str = get_name((char*)str_post, (char*)packet, &i);
				if(p_str && i < GET_NAME_LEN)
				{
					memcpy(web_cfg.get_name, p_str, i);
					web_cfg.get_name[i] = 0;
					if(strcmp((char*)str_update_page, web_cfg.get_name) == 0)
					{
						/*
						web_cfg.is_update_post = 1;
						web_cfg.update_data_cnt = 0;
						web_cfg.update_data_timer = 0;
						web_cfg.update_addr = UPDATE_DATA_BASE_ADDR;
						web_cfg.update_remain_size = 0;
						erase_update_erea(UPDATE_DATA_BASE_ADDR, UPDATE_DATE_EREA_SIZE);*/
						p_err_miss;
					}					
				}else
					web_cfg.get_name[0] = 0;

				p_str = get_head_tag(str_content_len, (const char*)packet, &i);
				if(p_str && i < 8)
				{
					char tmp[8];
					memcpy(tmp, p_str, 8);
					tmp[i] = 0;
					web_cfg.post_len = atoi(tmp);
                                        p_dbg("post len:%d", web_cfg.post_len);
					
				}
			}

                        p_dbg(web_cfg.get_name);

			if(web_cfg.web_sm > WEB_SM_IDLE){
				goto again;
			}
			break;
		//如果get请求数据没接收完，进入此模式(get请求只有head部分)
		case WEB_SM_GET_GETMORE: 
			if(strstr((const char*)packet, str_sect_end))
			{
				web_cfg.web_sm += 1;
				goto again;
			} else if(is_str_end(packet[0] ) && web_cfg.str_end_len) {
			 	int len = 1;
			 	if(is_str_end(packet[1] ))
					len++;
				if(is_str_end(packet[2] ))
					len++;
				if(is_str_end(packet[3] ))
					p_err("?");

				if((len + web_cfg.str_end_len) >= 4){
					web_cfg.web_sm += 1;
					goto again;
				}
			 }else if(is_str_end(packet[packetlength - 1])){
				web_cfg.str_end_len = 1;
				if(is_str_end(packet[packetlength - 2]))
					web_cfg.str_end_len ++;
				if(is_str_end(packet[packetlength - 3]))
					web_cfg.str_end_len ++;
                                p_dbg("str half end:%d", web_cfg.str_end_len);
			}else
				web_cfg.str_end_len = 0;
	
			break;
		//如果post请求数据没接收完，进入此模式。
		//上传大数据(比如升级固件)也是通过post方式。
		case WEB_SM_POST_GETMORE: 
		{
			int remain;
			if(web_cfg.post_len == 0){ //
				p_str = get_head_tag(str_content_len, (const char*)packet, &i);
				if(p_str && i < 8)
				{
					char tmp[8];
					memcpy(tmp, p_str, 8);
					tmp[i] = 0;
					web_cfg.post_len = atoi(tmp);
                                        p_dbg("post len1:%d", web_cfg.post_len);
				}
			}

			p_str = strstr((const char*)packet, str_sect_end);
			if(p_str)
			{
                                p_dbg("0:%s", p_str);
				p_str += SIZEOF_STR(str_sect_end);
                                p_dbg("1:%d, %s", SIZEOF_STR(str_sect_end), p_str);
				goto find_end;
			}else if(is_str_end(packet[0] ) && web_cfg.str_end_len) {
			 	int len = 1;
			 	if(is_str_end(packet[1] ))
					len++;
				if(is_str_end(packet[2] ))
					len++;
				if(is_str_end(packet[3] ))
					p_err("?");

				if((len + web_cfg.str_end_len) >= 4){
					
                                        p_dbg("0:%s", packet);
					p_str = (char*)packet + len;
                                        p_dbg("1:%s", p_str);
					goto find_end;
				}
			 }else if(is_str_end(packet[packetlength - 1])){
				web_cfg.str_end_len = 1;
				if(is_str_end(packet[packetlength - 2]))
					web_cfg.str_end_len ++;
				if(is_str_end(packet[packetlength - 3]))
					web_cfg.str_end_len ++;
                                p_dbg("str half end:%d", web_cfg.str_end_len);
			}else
				web_cfg.str_end_len = 0;
			break;
find_end:
			if(web_cfg.post_len == 0)
				web_cfg.web_sm = WEB_SM_RESPONE_POST;
			else{
				web_cfg.web_sm = WEB_SM_POST_GETMORE1;
				
				remain = packetlength + (uint32_t)packet - (uint32_t)p_str;
				if(((uint32_t)(packetlength + packet) > (uint32_t)p_str) && ((remain) < WEB_BUFF_MAX_VALID))
				{	
					if(web_cfg.is_update_post)
					{
						if(write_update_data((uint8_t*)p_str, remain) == -1)
						{
							p_err("write_update_data err");
							web_cfg.is_update_post = 0;
							web_cfg.update_data_cnt = 0;
							web_cfg.web_sm =  WEB_SM_IDLE;
						}
						web_cfg.update_data_cnt += remain; 
					}else{
						memcpy(web_buff, p_str, remain);
						web_buff[remain] = 0;
					}
                                        p_dbg("post remain:%d", remain);
				}else{
					p_err("plen:%d, remanlen:%d", packetlength, remain);
					memset(web_buff, 0, WEB_BUFF_SIZE);
				}

				packetlength = 0; //数据已经读到
			}
                        p_dbg("post more next:%d", web_cfg.web_sm);
			goto again;
//			break;
		}
		//这里主要接收post请求的数据部分
		case WEB_SM_POST_GETMORE1:
			if(web_cfg.is_update_post) //update data
			{
				write_update_data(packet, packetlength);
				web_cfg.update_data_cnt += packetlength; 
				web_cfg.update_data_timer = 0;
				if(web_cfg.update_data_cnt >= web_cfg.post_len)
				{
					write_update_data_finish();
					web_cfg.web_sm += 1;
					goto again;
				}
				
			}else{
				i = strlen(web_buff);
				packetlength = ((packetlength + i) < WEB_BUFF_MAX_VALID)?packetlength:(WEB_BUFF_MAX_VALID - i);
				memcpy(web_buff, packet, packetlength);
				if(i + packetlength >= web_cfg.post_len) 
				{
                                        p_dbg("post data end:%s", web_buff);
					web_cfg.web_sm += 1;
					goto again;
				}
			}
			break;
		//应答get请求，(发送页面)
		case WEB_SM_RESPONE_GET:
                        p_dbg("RESPONE_GET:%s", web_cfg.get_name);
			web_cfg.cur_routing = 0;
			web_cfg.web_sm = WEB_SM_IDLE; //web_sm在handle_web_get根据情况进行改变
			handle_web_get(s);
			if(web_cfg.web_sm == WEB_SM_SEND_DATA)
				goto again;
			break;
		//处理post请求，主要处理post过来的数据
		case WEB_SM_RESPONE_POST:
                        p_dbg("RESPONE_POST");
			web_cfg.cur_routing = 0;
			web_cfg.web_sm = WEB_SM_IDLE;//同上， 必须放在handle_web_post前面
			handle_web_post(s);
			if(web_cfg.web_sm == WEB_SM_SEND_DATA)
				goto again;
			break;
		//发送数据状态(发送页面)
		case WEB_SM_SEND_DATA:
			while(web_cfg.cur_routing)
			{
				web_cfg.cur_routing(s, -1);
			}

			web_cfg.web_sm = WEB_SM_IDLE;
			//if(web_cfg.need_reboot)
				//Sys_reboot();
			break;
		default:
			web_cfg.web_sm = WEB_SM_IDLE;
			break;
	}

	
	return 0;
}



void web_server_thread(void *arg)
{
	int socket_s = (int)arg;
	int ret, size;
	struct sockaddr addr;
	
	while(1)
	{
		size = sizeof(struct sockaddr);
		ret = accept(socket_s, &addr, (socklen_t*)&size);
		if(ret == -1)
		{
                        p_err("accept error");
			sleep(1000);
		}			
	}
}

void web_server_init(void)
{
	int  socket_s, err;
	struct sockaddr_in serv;
	memset(&serv, 0, sizeof(struct sockaddr_in));
	
	memset(&web_cfg, 0, sizeof(WEB_CFG));
	web_cfg.web_sm = WEB_SM_IDLE;

	serv.sin_family = AF_INET;
	serv.sin_port = htons(80);	
	serv.sin_addr.s_addr = htons(INADDR_ANY);

	mutex_lock(socket_mutex);		
	socket_s = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_s == -1)
	{
		p_err("%s err1", __FUNCTION__);
		mutex_unlock(socket_mutex);
		return;
	}
	err = bind(socket_s, (struct sockaddr*)&serv, sizeof(struct sockaddr_in));
	if(err == -1)
	{
		p_err("%s err2", __FUNCTION__);
		mutex_unlock(socket_mutex);
		return;
	}
	err = listen(socket_s, 4);
	if(err == -1)
	{
		p_err("%s err3", __FUNCTION__);
		mutex_unlock(socket_mutex);
		return;
	}
	mutex_unlock(socket_mutex);	

	thread_create(web_server_thread, (void*)socket_s, TASK_WEB_PRIO, 0, TASK_WEB_STK_SIZE, "tcp_recv_thread");
}

#endif

