/*
 * SPDX-FileCopyrightText: 2026 Leonard Herbst
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 *
 * @file
 * @brief   Host/VM layout check (optional, module virtwip_layout_check)
 *
 * Host and VM compile the same headers with different compilers, targets
 * and pointer widths. Structures accessed from both sides must still have
 * identical layouts. With VIRTWIP_LAYOUT_CHECK enabled, the VM entry point
 * virtwip_layout_info() reports sizeof/offsetof of everything listed in
 * virtwip_layout_list.h, and virtwip_vm_start() compares the values with
 * the host's own and refuses to start the VM on any mismatch.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "lwip/opt.h"
#include "lwip/err.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#if LWIP_NETCONN
#include "lwip/api.h"
#include "lwip/netbuf.h"
#endif

#include "shared.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Maximum number of checked layout properties
 */
#define VIRTWIP_LAYOUT_MAX  (256)

/**
 * @brief   Argument bundle of the virtwip_layout_info() VM entry point
 *
 * The VM writes one value per entry of virtwip_layout_list.h (in list
 * order) and returns the number of entries.
 */
typedef struct {
    uint32_t values[VIRTWIP_LAYOUT_MAX];
} virtwip_layout_args_t;

#ifdef __cplusplus
}
#endif

/** @} */
