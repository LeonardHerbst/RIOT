/*
 * SPDX-FileCopyrightText: 2026 Leonard Herbst
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 *
 * @file
 * @brief   Host side of the host/VM layout check
 *
 * @author  Leonard Herbst <leonard.herbst@tu-dresden.de>
 */

#include "lwip/opt.h"

#if VIRTWIP_LAYOUT_CHECK

#include <stdint.h>
#include <stdio.h>

#include "flow_pool.h"
#include "virtwip_layout.h"

static const char *const _names[] = {
#define VIRTWIP_LAYOUT_SIZE(type)           "sizeof(" #type ")",
#define VIRTWIP_LAYOUT_OFFSET(type, member) "offsetof(" #type ", " #member ")",
#include "virtwip_layout_list.h"
#undef VIRTWIP_LAYOUT_SIZE
#undef VIRTWIP_LAYOUT_OFFSET
};

static const uint32_t _host_values[] = {
#define VIRTWIP_LAYOUT_SIZE(type)           (uint32_t)sizeof(type),
#define VIRTWIP_LAYOUT_OFFSET(type, member) (uint32_t)offsetof(type, member),
#include "virtwip_layout_list.h"
#undef VIRTWIP_LAYOUT_SIZE
#undef VIRTWIP_LAYOUT_OFFSET
};

#define _NUMOF  (sizeof(_host_values) / sizeof(_host_values[0]))

_Static_assert(_NUMOF <= VIRTWIP_LAYOUT_MAX,
               "virtwip_layout_list.h has more entries than VIRTWIP_LAYOUT_MAX");

/* static: 1 KiB, passed to the VM as argument region */
static virtwip_layout_args_t _vm_values;

int virtwip_layout_check(void)
{
    int64_t res;

    if (virtwip_flow_call("virtwip_layout_info", &_vm_values, sizeof(_vm_values),
                          &res) != 0) {
        printf("virtwip: layout check: calling the VM failed\n");
        return -1;
    }
    if ((uint32_t)res != _NUMOF) {
        printf("virtwip: layout check: VM reports %u entries, host %u\n",
               (unsigned)(uint32_t)res, (unsigned)_NUMOF);
        return -1;
    }

    unsigned mismatches = 0;
    for (unsigned i = 0; i < _NUMOF; i++) {
        if (_vm_values.values[i] != _host_values[i]) {
            printf("virtwip: layout mismatch: %s: host %u, VM %u\n", _names[i],
                   (unsigned)_host_values[i], (unsigned)_vm_values.values[i]);
            mismatches++;
        }
    }
    if (mismatches) {
        printf("virtwip: layout check: %u of %u entries differ\n",
               mismatches, (unsigned)_NUMOF);
        return -1;
    }
    return 0;
}

#endif /* VIRTWIP_LAYOUT_CHECK */

/** @} */
