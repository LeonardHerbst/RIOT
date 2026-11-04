#include "lwip/netif.h"
#include "lwip/etharp.h"

#if MODULE_SLIPDEV_NET && LWIP_IPV4
static err_t (*bpf_call_slip_output4)(netif_t *, pbuf_t *, const ip4_addr_t *) = (void *) BPF_FUNC_CALL_SLIP_OUTPUT4;
err_t netif_null_output_ip4(netif_t *netif, pbuf_t *p, const ip4_addr_t *ipaddr);
#endif /* MODULE_SLIPDEV_NET && LWIP_IPV4 */
#if VIRTWIP_ENABLE_SLIPIF
err_t slipif_output_v4(netif_t *netif, pbuf_t *p, const ip4_addr_t *ipaddr);
#endif /* VIRTWIP_ENABLE_SLIPIF */

err_t netif_output_fn_dispatch(netif_output_fn fn, netif_t *netif, pbuf_t *p, const ip4_addr_t *ipaddr)
{
    switch (fn) {
#if LWIP_ARP
        case NETIF_OUTPUT_FN_ETHARP_OUTPUT:
            return etharp_output(netif, p, ipaddr);
#endif /* LWIP_ARP */
#if VIRTWIP_ENABLE_SLIPIF
        case NETIF_OUTPUT_FN_SLIP_OUTPUT_V4:
            return slipif_output_v4(netif, p, ipaddr);
#endif /* VIRTWIP_ENABLE_SLIPIF */
#if LWIP_IPV4
        case NETIF_OUTPUT_FN_NULL_OUTPUT_IP4:
            return netif_null_output_ip4(netif, p, ipaddr);
#endif /* LWIP_IPV4 */
#if LWIP_IPV4 && MODULE_SLIPDEV_NET
        case NETIF_OUTPUT_FN_EXTERNAL_SLIP_OUTPUT4:
            return bpf_call_slip_output4(netif, p, ipaddr);
#endif /* if LWIP_IPV4 && MODULE_SLIPDEV_NET */
        /* NETIF_OUTPUT_FN_LOOP_OUTPUT_IPV4 is reserved (loopback support is
         * disabled in this build); falls through to the abort below. */
        default:
            LWIP_ASSERT("ICD ERROR illegal netif_output_fn!\n", false);
            return 0;
    }
}
