#include "lwip/priv/tcpip_priv.h"

#if LWIP_NETIF_API
err_t netifapi_do_netif_add(tcpip_api_call_data_t *m);
err_t netifapi_do_netif_set_addr(tcpip_api_call_data_t *m);
err_t netifapi_do_netif_common(tcpip_api_call_data_t *m);
err_t netifapi_do_name_to_index(tcpip_api_call_data_t *m);
err_t netifapi_do_index_to_name(tcpip_api_call_data_t *m);
#endif /* LWIP_NETIF_API */

err_t tcpip_api_call_fn_dispatch(tcpip_api_call_fn fn, tcpip_api_call_data_t *call)
{
    switch (fn) {
#if LWIP_NETIF_API
        case TCPIP_API_CALL_FN_NETIFAPI_DO_NETIF_ADD:
            return netifapi_do_netif_add(call);
        case TCPIP_API_CALL_FN_NETIFAPI_DO_NETIF_SET_ADDR:
            return netifapi_do_netif_set_addr(call);
        case TCPIP_API_CALL_FN_NETIFAPI_DO_NETIF_COMMON:
            return netifapi_do_netif_common(call);
        case TCPIP_API_CALL_FN_NETIFAPI_DO_NAME_TO_INDEX:
            return netifapi_do_name_to_index(call);
        case TCPIP_API_CALL_FN_NETIFAPI_DO_INDEX_TO_NAME:
            return netifapi_do_index_to_name(call);
#endif /* LWIP_NETIF_API */
        default:
            LWIP_ASSERT("ICD ERROR illegal tcpip_api_call_fn!\n", false);
            return 0;
    }
}
