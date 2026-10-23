#include "lwip/netifapi.h"

err_t netifapi_errt_fn_dispatch(netifapi_errt_fn fn, netif_t *netif)
{
    switch (fn) {
#if LWIP_DHCP
        case NETIFAPI_ERRT_FN_DHCP_START:
            return dhcp_start(netif);
        case NETIFAPI_ERRT_FN_DHCP_RENEW:
            return dhcp_renew(netif);
        case NETIFAPI_ERRT_FN_DHCP_RELEASE:
            return dhcp_release(netif);
#endif /* LWIP_DHCP */
#if LWIP_AUTOIP
        case NETIFAPI_ERRT_FN_AUTOIP_START:
            return autoip_start(netif);
        case NETIFAPI_ERRT_FN_AUTOIP_STOP:
            return autoip_stop(netif);
#endif /* LWIP_AUTOIP */
        default:
            LWIP_ASSERT("ICD ERROR illegal netifapi_errt_fn!\n", false);
            return 0;
    }
}
