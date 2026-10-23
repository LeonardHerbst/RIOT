#include "lwip/netifapi.h"

void netifapi_void_fn_dispatch(netifapi_void_fn fn, netif_t *netif)
{
    switch (fn) {
        case NETIFAPI_VOID_FN_NETIF_REMOVE:
            netif_remove(netif);
            break;
        case NETIFAPI_VOID_FN_NETIF_SET_UP:
            netif_set_up(netif);
            break;
        case NETIFAPI_VOID_FN_NETIF_SET_DOWN:
            netif_set_down(netif);
            break;
        case NETIFAPI_VOID_FN_NETIF_SET_DEFAULT:
            netif_set_default(netif);
            break;
        case NETIFAPI_VOID_FN_NETIF_SET_LINK_UP:
            netif_set_link_up(netif);
            break;
        case NETIFAPI_VOID_FN_NETIF_SET_LINK_DOWN:
            netif_set_link_down(netif);
            break;
#if LWIP_DHCP
        case NETIFAPI_VOID_FN_DHCP_STOP:
            dhcp_stop(netif);
            break;
        case NETIFAPI_VOID_FN_DHCP_INFORM:
            dhcp_inform(netif);
            break;
        case NETIFAPI_VOID_FN_DHCP_RELEASE_AND_STOP:
            dhcp_release_and_stop(netif);
            break;
#endif /* LWIP_DHCP */
        default:
            LWIP_ASSERT("ICD ERROR illegal netifapi_void_fn!\n", false);
            break;
    }
}
