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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "femtocontainer/femtocontainer.h"

#include "flow_pool.h"
#include "lwip.h"
#include "virtwip_config.h"
#include "blob/lwip.bin.h"  /* VM image blob */

static f12r_t _vm;

void virtwip_vm_start(void)
{
    _vm = (f12r_t){
        .application = lwip_bin,
        .application_len = lwip_bin_len,
        /* No buffer given: femto malloc()s the VM's .data + .bss region
         * (globals and the lwIP heap) in f12r_setup() */
        .mutable_data = NULL,
        .mutable_data_len = 0,
    };

    f12r_setup(&_vm);
    int res = f12r_verify_preflight(&_vm);
    if (res != FC_OK) {
        printf("virtwip: VM image failed verification (%d)\n", res);
        assert(false);
        return;
    }

    virtwip_flow_pool_init(&_vm);

#if VIRTWIP_LAYOUT_CHECK
    if (virtwip_layout_check() != 0) {
        printf("virtwip: host/VM layouts differ, VM disabled\n");
        virtwip_flow_pool_disable();
        assert(false);
    }
#endif
}

/** @} */
