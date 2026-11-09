/*
 * SPDX-FileCopyrightText: 2021 Google LLC
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup sys_auto_init_lwip_netif
 * @{
 *
 * @file
 * @brief       Initializes the supported network interfaces for lwIP
 * @author      Erik Ekman <eekman@google.com>
 */

#include "kernel_defines.h"
#include "lwip.h"
#include "lwip_init_devs.h"
#include "lwip/tcpip.h"
#include "lwip/netif/compat.h"
#include "lwip/netif/netdev.h"
#include "netif/lowpan6.h"
#include "flow_pool.h"
#include "xfa.h"

XFA_INIT_CONST(lwip_netif_setup_func_t, lwip_netif_eth_xfa);
XFA_INIT_CONST(lwip_netif_setup_func_t, lwip_netif_6lowpan_xfa);

/**
 * @brief   Initializes network interfaces
 */
void lwip_netif_init_devs(void)
{
    /* TODO: do for every eligible netdev */

    /* Ethernet interfaces
     * ------------------- */

    int i;
    const int eth_devs = XFA_LEN(lwip_netif_setup_func_t, lwip_netif_eth_xfa);
    for (i = 0; i < eth_devs; i++) {
        lwip_netif_eth_xfa[i]();
    }

    /* 6LoWPAN interfaces
     * ------------------ */

    const int sixlowpan_devs = XFA_LEN(lwip_netif_setup_func_t, lwip_netif_6lowpan_xfa);
    for (i = 0; i < sixlowpan_devs; i++) {
        lwip_netif_6lowpan_xfa[i]();
    }
}

static struct netif *setup_netif(lwip_netif_t *netif, netdev_t *state,
                                 netif_input_fn input_fn)
{
    state->context = netif;
    /* lwIP in the VM works on this host-owned struct netif from now on
     * (netif_add, netif list, RX/TX): make it accessible to all flows.
     * TODO: allocate the netif in VM memory instead (no host memory exposed). */
    virtwip_vm_add_region(&netif->vm_region, &netif->lwip_netif, sizeof(netif->lwip_netif));
    /* RIOT-LWIP-BORDER: lwIP call | also hands lwip_netdev_init() to lwIP
     * as netif->init callback (lwIP calls back into RIOT from netif_add)
     * and lwIP's input_fn (tcpip_input/tcpip_6lowpan_input) as netif->input */
    virtwip_netif_add_noaddr_args_t args = {
        .netif = (bpfptr_t)(uintptr_t)&netif->lwip_netif,
        .state = (bpfptr_t)(uintptr_t)state,
        .init = (bpfptr_t)(uintptr_t)lwip_netdev_init,
        .input = (uint32_t)input_fn,
    };
    int64_t res;
    if (virtwip_flow_call("virtwip_netif_add_noaddr", &args, sizeof(args), &res) != 0) {
        return NULL;
    }
    struct netif *_if = (struct netif *)(uintptr_t)res;
    if (_if) {
        netif_register(&netif->common_netif);
    }
    return _if;
}

struct netif *lwip_add_ethernet(lwip_netif_t *netif, netdev_t *state)
{
    struct netif *_if = setup_netif(netif, state, NETIF_INPUT_FN_TCPIP_INPUT);
    if (_if) {
        /* RIOT-LWIP-BORDER: lwIP call | only takes effect if no default is
         * set yet (checked VM-side) */
        virtwip_netif_args_t args = { .netif = (bpfptr_t)(uintptr_t)_if };
        int64_t res;
        virtwip_flow_call("virtwip_netif_set_default", &args, sizeof(args), &res);
    }
    return _if;
}

#if IS_USED(MODULE_VIRTWIP_SIXLOWPAN)
struct netif *lwip_add_6lowpan(lwip_netif_t *netif, netdev_t *state)
{
    return setup_netif(netif, state, NETIF_INPUT_FN_TCPIP_6LOWPAN_INPUT);
}
#endif /* MODULE_VIRTWIP_SIXLOWPAN */

/**@}*/
