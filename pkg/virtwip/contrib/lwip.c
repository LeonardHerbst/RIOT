/*
 * SPDX-FileCopyrightText: 2019-2021 Freie Universität Berlin
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 *
 * @file
 * @author Martine Lenders <mlenders@inf.fu-berlin.de>
 * @author Erik Ekman <eekman@google.com>
 */

#include "kernel_defines.h"

#include "lwip/tcpip.h"
#include "lwip/netif/netdev.h"
#include "lwip/netif.h"
#include "lwip/netifapi.h"
#include "netif/lowpan6.h"

#include "lwip.h"
#include "lwip_init_devs.h"
#include "flow_pool.h"

#define ENABLE_DEBUG    0
#include "debug.h"

void lwip_bootstrap(void)
{
    /* RIOT-LWIP-BORDER: RIOT => VM | start the VM */
    virtwip_vm_start();
    lwip_netif_init_devs();
    /* RIOT-LWIP-BORDER: lwIP call | spawns tcpip_thread (via sys_thread_new) */
    virtwip_tcpip_init_args_t args = { 0 };
    int64_t res;
    virtwip_flow_call("virtwip_tcpip_init", &args, sizeof(args), &res);
#if IS_USED(MODULE_VIRTWIP_DHCP_AUTO)
    /* Start DHCP on all supported netifs. Interfaces that support
     * link status events will reset DHCP retries when link comes up. */
    virtwip_flow_call("virtwip_dhcp_start_all", NULL, 0, &res);
#endif
}

/** @} */
