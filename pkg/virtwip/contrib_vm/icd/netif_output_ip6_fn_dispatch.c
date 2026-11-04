#include "lwip/netif.h"
#include "lwip/ethip6.h"
#include "netif/lowpan6.h"
#include "netif/lowpan6_ble.h"
#include "netif/slipif.h"

#if LWIP_IPV6
#if MODULE_SLIPDEV_NET
static err_t (*bpf_call_slip_output6)(netif_t *, pbuf_t *, const ip6_addr_t *) = (void *) BPF_FUNC_CALL_SLIP_OUTPUT6;
#endif /* MODULE_SLIPDEV_NET */
err_t netif_null_output_ip6(netif_t *netif, pbuf_t *p, const ip6_addr_t *ipaddr);
#if VIRTWIP_ENABLE_SLIPIF
err_t slipif_output_v6(netif_t *netif, pbuf_t *p, const ip6_addr_t *ipaddr);
#endif /* VIRTWIP_ENABLE_SLIPIF */

err_t netif_output_ip6_fn_dispatch(netif_output_ip6_fn fn, netif_t *netif, pbuf_t *p, const ip6_addr_t *ipaddr)
{
    switch (fn) {
        case NETIF_OUTPUT_IP6_FN_ETHIP6_OUTPUT:
            return ethip6_output(netif, p, ipaddr);
        case NETIF_OUTPUT_IP6_FN_NULL_OUTPUT_IP6:
            return netif_null_output_ip6(netif, p, ipaddr);
        case NETIF_OUTPUT_IP6_FN_LOWPAN6_OUTPUT:
            return lowpan6_output(netif, p, ipaddr);
        case NETIF_OUTPUT_IP6_FN_RFC7668_OUTPUT:
            return rfc7668_output(netif, p, ipaddr);
#if VIRTWIP_ENABLE_SLIPIF
        case NETIF_OUTPUT_IP6_FN_SLIP_OUTPUT_V6:
            return slipif_output_v6(netif, p, ipaddr);
#endif /* VIRTWIP_ENABLE_SLIPIF */
#if MODULE_SLIPDEV_NET
        case NETIF_OUTPUT_IP6_FN_EXTERNAL_SLIP_OUTPUT6:
            return bpf_call_slip_output6(netif, p, ipaddr);
#endif /* MODULE_SLIPDEV_NET */
        /* NETIF_OUTPUT_IP6_FN_LOOP_OUTPUT_IPV6 is reserved (loopback support
         * is disabled in this build); falls through to the abort below. */

        default:
            LWIP_ASSERT("ICD ERROR illegal netif_output_ip6_fn!\n", false);
            return 0;
    }
}
#endif /* LWIP_IPV6 */
