#include "lwip/udp.h"

void recv_udp(void *arg, udp_pcb_t *pcb, pbuf_t *p, const ip_addr_t *addr, u16_t port);
void zepif_udp_recv(void *arg, udp_pcb_t *pcb, pbuf_t *p, const ip_addr_t *addr, u16_t port);

void udp_recv_fn_dispatch(udp_recv_fn fn, udp_pcb_t *pcb, pbuf_t *p, const ip_addr_t *addr, u16_t port)
{
    switch (fn) {
#if LWIP_NETCONN
        case UDP_RECV_FN_RECV_UDP:
            recv_udp(pcb->recv_arg, pcb, p, addr, port);
            break;
#endif /* LWIP_NETCONN */
#if LWIP_IPV6 && LWIP_UDP
        case UDP_RECV_FN_ZEP_IF_UDP_RECV:
            zepif_udp_recv(pcb->recv_arg, pcb, p, addr, port);
            break;
#endif /* LWIP_IPV6 && LWIP_UDP */
        default:
            LWIP_ASSERT("ICD ERROR illegal udp_recv_fn!\n", false);
            break;
    }
}
