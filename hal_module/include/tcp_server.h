/*
 * tcp_server.h
 *
 *  Created on: 18 Haz 2023
 *      Author: user
 */

#ifndef SRC_HAL_MODULE_INCLUDE_TCP_SERVER_H_
#define SRC_HAL_MODULE_INCLUDE_TCP_SERVER_H_

//#include "lwipopts.h"
//#include "lwip/tcp.h"
//#include "lwip/ip_addr.h"

//#include "lwip/sockets.h"
//#include "xil_printf.h"
//#include "lwip/netbuf.h"

#include "xparameters.h"

#include "netif/xadapter.h"
#include "lwip/inet.h"
#include "lwip/api.h"
#include "lwip/init.h"
#include "string.h"


#if LWIP_DHCP==1
#include "lwip/dhcp.h"
extern volatile int dhcp_timoutcntr;
err_t dhcp_start(struct netif *netif);
#endif

struct p_buf{
  char buf[100];
  u32 lenth;
};


#define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_0_BASEADDR

#define DEFAULT_IP_ADDRESS "192.168.1.10"
#define DEFAULT_IP_MASK "255.255.255.0"
#define DEFAULT_GW_ADDRESS "192.168.1.1"

#define RECV_BUF_SIZE 1500

#define TCP_SERVER_THREAD_STACKSIZE 2048

/* server port to listen on/connect to */
#define TCP_CONN_PORT 5001

/* seconds between periodic bandwidth reports */
#define INTERIM_REPORT_INTERVAL 5

#define THREAD_STACKSIZE 1024



void tcp_recv_(void *arg);
void tcp_thread(void *arg);
void print_ip(char *msg, ip_addr_t *ip);
void print_ip_settings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw);
void assign_default_ip(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw);
void main_thread(void *p);
void tcp_server_init(void);

#endif /* SRC_HAL_MODULE_INCLUDE_TCP_SERVER_H_ */
