/*
 * SPDX-FileCopyrightText: 2015 Martine Lenders <mlenders@inf.fu-berlin.de>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @ingroup     pkg_lwip
 *
 * @{
 *
 * @file
 * @brief   lwIP bootstrap definitions
 *
 * @author  Martine Lenders <mlenders@inf.fu-berlin.de>
 */

#include "event.h"
#include "femtocontainer/femtocontainer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief event queue for netdev events
 */
extern event_queue_t lwip_event_queue;

/**
 * @brief   Initializes lwIP stack.
 *
 * This initializes lwIP, i.e. all netdevs are added to as interfaces to the
 * stack and the stack's thread is started.
 */
void lwip_bootstrap(void);

/**
 * @brief   Loads and starts the lwIP VM
 *
 * Sets up the VM image, validates it and initializes the flow pool. Called
 * by @ref lwip_bootstrap.
 */
void virtwip_vm_start(void);

/**
 * @brief   Makes host memory accessible to every VM flow
 *
 * Only for host-owned objects lwIP itself works on (e.g. a netif). The
 * region list is not protected against concurrent executions: call before
 * other VM executions can run, i.e. during bootstrap.
 *
 * @param[out] region   region node, must stay valid as long as the VM runs
 * @param[in]  start    start of the memory
 * @param[in]  len      length in bytes
 */
void virtwip_vm_add_region(f12r_mem_region_t *region, void *start, size_t len);

/**
 * @brief   Compares the layouts of all structures shared between host and
 *          VM (only with module virtwip_layout_check)
 *
 * @return 0 if all layouts match
 * @return -1 on a mismatch (details are printed) or if the VM call failed
 */
int virtwip_layout_check(void);

#ifdef __cplusplus
}
#endif

/** @} */
