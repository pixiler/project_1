/*
 * tcp_server.c
 *
 *  Created on: 18 Haz 2023
 *      Author: user
 */


#include "tcp_server.h"

/* Interval time in seconds */
#define REPORT_INTERVAL_TIME (INTERIM_REPORT_INTERVAL * 1000)

struct netif server_netif;

static int complete_nw_thread;

struct netbuf *buf;
struct recv_buf *recv_msg;

static sys_thread_t main_thread_handle;
static sys_thread_t recv_thread_handle;
static sys_thread_t transmit_thread_handle;
static sys_thread_t tcp_thread_handle;


static int recv ,len;


void print_ip(char *msg, ip_addr_t *ip)
{
	xil_printf(msg);
	xil_printf("%d.%d.%d.%d\n\r", ip4_addr1(ip), ip4_addr2(ip),
				ip4_addr3(ip), ip4_addr4(ip));
}

void print_ip_settings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{
	print_ip("Board IP:       ", ip);
	print_ip("Netmask :       ", mask);
	print_ip("Gateway :       ", gw);
}

void assign_default_ip(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{
	int err;

	xil_printf("Configuring default IP %s \r\n", DEFAULT_IP_ADDRESS);

	err = inet_aton(DEFAULT_IP_ADDRESS, ip);
	if(!err)
		xil_printf("Invalid default IP address: %d\r\n", err);

	err = inet_aton(DEFAULT_IP_MASK, mask);
	if(!err)
		xil_printf("Invalid default IP MASK: %d\r\n", err);

	err = inet_aton(DEFAULT_GW_ADDRESS, gw);
	if(!err)
		xil_printf("Invalid default gateway address: %d\r\n", err);
}

static struct netconn *conn, *newconn;
static ip_addr_t *addr;
static u16_t port;

extern char msg[100];
extern char smsg[100];

void tcp_server_recv()
{

  while(1)
    {
      /* receive the data from the client */
        while(netconn_recv(newconn, &buf) == ERR_OK)
          {
            /* Extrct the address and port in case they are required */
            addr = netbuf_fromaddr(buf);
            port = netbuf_fromport(buf);

            /* If there is some data remaining to be sent, the following process will continue */
            do
      	{
      	  strncpy(msg, buf->p->payload, buf->p->len); // get the message from the client

      	  /***/
      	  // Or modify the message received, so that we can send it back to the client
      	//smsg->lenth = sprintf (smsg->buf, "\"%s\" was sent by the Server\n", msg->buf);

      	recv = 1;
      	memset (msg, '\0', 100);  // clear the buffer
      	  /***/
      	}
            while(netbuf_next(buf)>0);

            netbuf_delete(buf);
          }

        /* Close connection and discard connection identifier. */
        netconn_close(newconn);
        netconn_delete(newconn);

        vTaskResume(tcp_thread_handle);
        vTaskSuspend(transmit_thread_handle);
        vTaskSuspend(recv_thread_handle);


    }


}

void tcp_server_transmit()
{
  while(1)
    {

      if(recv)
          {
            if(newconn->state != NETCONN_INVALID);
                netconn_write(newconn, smsg, len, NETCONN_COPY);  // send the message back to the client

                recv = 0;
          }
    }


}

void tcp_thread(void *arg)
{
  err_t error, accept_err;

  /* Create a new connection identifier. */
  conn = netconn_new(NETCONN_TCP);


  if(conn != NULL)
    {
      /* Bind connection to the port number TCP_CONN_PORT. */
      error = netconn_bind(conn, IP_ADDR_ANY, TCP_CONN_PORT);

      if(error == ERR_OK)
	{
	  /* Tell connection to go into listening mode. */
	  netconn_listen(conn);

	  while(1)
	    {
	      /* Grab new connection. */
	      accept_err = netconn_accept(conn, &newconn);

	      /* Process the new connection. */
	      if(accept_err == ERR_OK)
		{
		  if(recv_thread_handle == NULL)
		    recv_thread_handle = sys_thread_new("tcp_recv", tcp_server_recv, 0, THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
		  else
		    vTaskResume(recv_thread_handle);

		  if(transmit_thread_handle == NULL)
		    transmit_thread_handle = sys_thread_new("tcp_transmit", tcp_server_transmit, 0, THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
		  else
		    vTaskResume(transmit_thread_handle);

		  vTaskSuspend(tcp_thread_handle);

		}
	    }
	}
      else
	{
	  netconn_delete(conn);
	}
    }

}
void network_thread(void *p)
{
#if (LWIP_DHCP==1)
	int mscnt = 0;
#endif

	/* the mac address of the board. this should be unique per board */
	u8_t mac_ethernet_address[] = { 0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };

	xil_printf("\n\r\n\r");
	xil_printf("-----lwIP Socket Mode TCP Server Application------\r\n");

	/* Add network interface to the netif_list, and set it as default */
	if (!xemac_add(&server_netif, NULL, NULL, NULL, mac_ethernet_address,
		PLATFORM_EMAC_BASEADDR)) {
		xil_printf("Error adding N/W interface\r\n");
		return;
	}

	netif_set_default(&server_netif);

	/* specify that the network if is up */
	netif_set_up(&server_netif);

	/* start packet receive thread - required for lwIP operation */
	sys_thread_new("xemacif_input_thread",
			(void(*)(void*))xemacif_input_thread, &server_netif,
			THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);

	complete_nw_thread = 1;

	/* Resume the main thread; auto-negotiation is completed */
	vTaskResume(main_thread_handle);

#if ((LWIP_IPV6==0) && (LWIP_DHCP==1))
	dhcp_start(&server_netif);
	while (1) {
		vTaskDelay(DHCP_FINE_TIMER_MSECS / portTICK_RATE_MS);
		dhcp_fine_tmr();
		mscnt += DHCP_FINE_TIMER_MSECS;
		if (mscnt >= DHCP_COARSE_TIMER_SECS*1000) {
			dhcp_coarse_tmr();
			mscnt = 0;
		}
	}
#else
	vTaskDelete(NULL);
#endif
}
void main_thread(void *p)
{
#if ((LWIP_IPV6==0) && (LWIP_DHCP==1))
	int mscnt = 0;
#endif

	/* initialize lwIP before calling sys_thread_new */
	lwip_init();

	/* any thread using lwIP should be created using sys_thread_new */
	sys_thread_new("nw_thread", network_thread, NULL,
			THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);

	/* Suspend Task until auto-negotiation is completed */
	if (!complete_nw_thread)
		vTaskSuspend(NULL);

#if LWIP_DHCP==1
	while (1) {
		vTaskDelay(DHCP_FINE_TIMER_MSECS / portTICK_RATE_MS);
		if (server_netif.ip_addr.addr) {
			xil_printf("DHCP request success\r\n");
			break;
		}
		mscnt += DHCP_FINE_TIMER_MSECS;
		if (mscnt >= 10000) {
			xil_printf("ERROR: DHCP request timed out\r\n");
			assign_default_ip(&(server_netif.ip_addr),
						&(server_netif.netmask),
						&(server_netif.gw));
			break;
		}
	}

#else
	assign_default_ip(&(server_netif.ip_addr), &(server_netif.netmask),
				&(server_netif.gw));
#endif

	print_ip_settings(&(server_netif.ip_addr), &(server_netif.netmask),
				&(server_netif.gw));

	xil_printf("\r\n");

	/* print all application headers */
	xil_printf("TCP server listening on port %d\r\n",
			TCP_CONN_PORT);
	//print_app_header();
	xil_printf("\r\n");

	/* start the application*/

	tcp_thread_handle = sys_thread_new("tcp_thred", tcp_thread, NULL,THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);

	vTaskDelete(NULL);
	//return;
}

void tcp_server_init(void)
{
      main_thread_handle = sys_thread_new("main_thread", main_thread, 0,
		      THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
}



