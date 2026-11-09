/*
 * SPDX-FileCopyrightText: 2026 Leonard Herbst
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 *
 * @file
 * @brief   lwIP VM (femto-container) startup
 *
 * @author  Leonard Herbst <leonard.herbst@tu-dresden.de>
 */

#include <assert.h>
#include <stdint.h>

#include "femtocontainer/femtocontainer.h"

#include "flow_pool.h"
#include "blob/lwip.bin.h"  /* VM image blob */

static f12r_t _vm;

void virtwip_vm_start(void)
{
    _vm = (f12r_t){
        .application = lwip_bin,
        .application_len = lwip_bin_len,
        /* femto allocates the VM's .data + .bss region (globals and heap) */
        .mut_regions = NULL,
        .mut_regions_len = 0,
        .branches_remaining = UINT32_MAX,
    };

    f12r_setup(&_vm);
    assert(f12r_verify_preflight(&_vm) == FC_OK);

    virtwip_flow_pool_init(&_vm);
}

/** @} */
