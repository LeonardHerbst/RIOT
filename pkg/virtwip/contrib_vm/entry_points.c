#include "shared.h"
#include "lwip/netif.h"

err_t virtwip_netif_input(virtwip_input_args_t *args)
{
    netif_t *netif = (netif_t *)(uintptr_t)args->netif;
    pbuf_t *p = (pbuf_t *)(uintptr_t)args->p;

    return netif_input_fn_dispatch(netif->input, p, netif);
}
