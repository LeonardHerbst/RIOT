#include "lwip/netif.h"
#include "lwip/tcpip.h"      /* tcpip_input */
#include "netif/lowpan6.h"   /* tcpip_6lowpan_input */

#if LWIP_IPV6 && LWIP_IPV4
err_t ip_input(pbuf_t *p, netif_t *inp);
#endif /* LWIP_IPV6 && LWIP_IPV4 */
#if LWIP_ARP || LWIP_ETHERNET
err_t ethernet_input(pbuf_t *p, netif_t *inp);
#endif /* LWIP_ARP || LWIP_ETHERNET */
err_t lowpan6_input(pbuf_t *p, netif_t *inp);
#if LWIP_IPV6
err_t rfc7668_input(pbuf_t *p, netif_t *inp);
#endif /* LWIP_IPV6 */
#if LWIP_NUM_NETIF_CLIENT_DATA
err_t bridgeif_input(pbuf_t *p, netif_t *inp);
#endif /* LWIP_NUM_NETIF_CLIENT_DATA */

err_t netif_input_fn_dispatch(netif_input_fn fn, pbuf_t *p, netif_t *inp)
{
    switch (fn) {
#if LWIP_IPV4 && LWIP_IPV6
        case NETIF_INPUT_FN_IP_INPUT:
            return ip_input(p, inp);
#endif /* LWIP_IPV4 && LWIP_IPV4*/
#if LWIP_ARP || LWIP_ETHERNET
        case NETIF_INPUT_FN_ETHERNET_INPUT:
            return ethernet_input(p, inp);
#endif /* LWIP_ARP || LWIP_ETHERNET */
#if LWIP_IPV6
        case NETIF_INPUT_FN_LOWPAN6_INPUT:
            return lowpan6_input(p, inp);
        case NETIF_INPUT_FN_RFC7668_INPUT:
            return rfc7668_input(p, inp);
#endif /* LWIP_IPV6 */
#if LWIP_NUM_NETIF_CLIENT_DATA
        case NETIF_INPUT_FN_BRIDGEIF_INPUT:
            return bridgeif_input(p, inp);
#endif /* LWIP_NUM_NETIF_CLIENT_DATA */
        case NETIF_INPUT_FN_TCPIP_INPUT:
            return tcpip_input(p, inp);
#if LWIP_IPV6
        case NETIF_INPUT_FN_TCPIP_6LOWPAN_INPUT:
            return tcpip_6lowpan_input(p, inp);
#endif /* LWIP_IPV6 */
        default:
            LWIP_ASSERT("ICD ERROR illegal netif_input_fn!\n", false);
            return 0;
    }
}
