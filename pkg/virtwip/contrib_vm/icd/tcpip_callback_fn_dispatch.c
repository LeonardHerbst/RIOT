#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#if LWIP_NETCONN
#include "lwip/priv/api_msg.h"
#endif /* LWIP_NETCONN */

void pbuf_free_int(void *p);
void pbuf_free_ooseq_callback(void *arg);

void tcpip_callback_fn_dispatch(tcpip_callback_fn fn, void *ctx)
{
    switch (fn) {
        case TCPIP_CALLBACK_FN_PBUF_FREE_INT:
            pbuf_free_int(ctx);
            break;
        case TCPIP_CALLBACK_FN_MEM_FREE:
            mem_free(ctx);
            break;
#if ENABLE_LOOPBACK
        case TCPIP_CALLBACK_FN_NETIF_POLL:
            netif_poll((virtwip_netif_t *)ctx);
            break;
#endif /* ENABLE_LOOPBACK */
#if LWIP_NETCONN
        case TCPIP_CALLBACK_FN_LWIP_NETCONN_DO_NEWCONN:
            lwip_netconn_do_newconn(ctx);
            break;
        case TCPIP_CALLBACK_FN_LWIP_NETCONN_DO_DELCONN:
            lwip_netconn_do_delconn(ctx);
            break;
        case TCPIP_CALLBACK_FN_LWIP_NETCONN_DO_GETADDR:
            lwip_netconn_do_getaddr(ctx);
            break;
        case TCPIP_CALLBACK_FN_LWIP_NETCONN_DO_BIND:
            lwip_netconn_do_bind(ctx);
            break;
        case TCPIP_CALLBACK_FN_LWIP_NETCONN_DO_BIND_IF:
            lwip_netconn_do_bind_if(ctx);
            break;
        case TCPIP_CALLBACK_FN_LWIP_NETCONN_DO_CONNECT:
            lwip_netconn_do_connect(ctx);
            break;
        case TCPIP_CALLBACK_FN_LWIP_NETCONN_DO_DISCONNECT:
            lwip_netconn_do_disconnect(ctx);
            break;
#if LWIP_TCP
        case TCPIP_CALLBACK_FN_LWIP_NETCONN_DO_LISTEN:
            lwip_netconn_do_listen(ctx);
            break;
#endif /* LWIP_TCP */
#if TCP_LISTEN_BACKLOG
        case TCPIP_CALLBACK_FN_LWIP_NETCONN_DO_ACCEPTED:
            lwip_netconn_do_accepted(ctx);
            break;
#endif /* TCP_LISTEN_BACKLOG */
#if LWIP_TCP
        case TCPIP_CALLBACK_FN_LWIP_NETCONN_DO_RECV:
            lwip_netconn_do_recv(ctx);
            break;
#endif /* LWIP_TCP */
        case TCPIP_CALLBACK_FN_LWIP_NETCONN_DO_SEND:
            lwip_netconn_do_send(ctx);
            break;
        case TCPIP_CALLBACK_FN_LWIP_NETCONN_DO_WRITE:
            lwip_netconn_do_write(ctx);
            break;
        case TCPIP_CALLBACK_FN_LWIP_NETCONN_DO_CLOSE:
            lwip_netconn_do_close(ctx);
            break;
#if LWIP_IGMP || (LWIP_IPV6 && LWIP_IPV6_MLD)
        case TCPIP_CALLBACK_FN_LWIP_NETCONN_DO_JOIN_LEAVE_GROUP:
            lwip_netconn_do_join_leave_group(ctx);
            break;
        case TCPIP_CALLBACK_FN_LWIP_NETCONN_DO_JOIN_LEAVE_GROUP_NETIF:
            lwip_netconn_do_join_leave_group_netif(ctx);
            break;
#endif /* LWIP_IGMP || (LWIP_IPV6 && LWIP_IPV6_MLD) */
#endif /* LWIP_NETCONN */
#if LWIP_DNS
        case TCPIP_CALLBACK_FN_LWIP_NETCONN_DO_GETHOSTBYNAME:
            lwip_netconn_do_gethostbyname(ctx);
            break;
#endif /* LWIP_DNS */
#if LWIP_TCP && TCP_QUEUE_OOSEQ && PBUF_POOL_FREE_OOSEQ
        case TCPIP_CALLBACK_FN_PBUF_FREE_OOSEQ_CALLBACK:
            pbuf_free_ooseq_callback(ctx);
            break;
#endif /* LWIP_TCP && TCP_QUEUE_OOSEQ && PBUF_POOL_FREE_OOSEQ */
        /* TCPIP_CALLBACK_FN_LWIP_GETSOCKOPT_CALLBACK / TCPIP_CALLBACK_FN_LWIP_SETSOCKOPT_CALLBACK
         * (sockets.c, LWIP_SOCKET) are reserved but have no dispatch cases: their
         * targets are static in sockets.c. Extend here if sockets support is added. */
        default:
            LWIP_ASSERT("ICD ERROR illegal tcpip_callback_fn!\n", false);
            break;
    }
}
