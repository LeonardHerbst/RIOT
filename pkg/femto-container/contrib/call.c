/*
 * Copyright (C) 2020 Inria
 * Copyright (C) 2020 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdint.h>
#include <stdio.h>

#include "femtocontainer/femtocontainer.h"
#include "femto_contrib.h"
#include "builtin_shared.h"


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"

uint32_t f12r_vm_printf(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    /* cast to uintptr_t first is necessary for <64 bit architectures
     * but it breaks the llu and lld formats */
    return printf((const char *)(uintptr_t)regs[1],
                  (uintptr_t)regs[2],
                  (uintptr_t)regs[3],
                  (uintptr_t)regs[4],
                  (uintptr_t)regs[5]);
}

uint32_t f12r_vm_abort(f12r_flow_t *flow, uint64_t *regs)
{
    (void) regs;
    flow->state |= FC_FLOW_STATE_ABORT;
    return 0;
}

uint32_t f12r_vm_abort_with_msg(f12r_flow_t *flow, uint64_t *regs)
{
    printf("%s", (char *)(uintptr_t)regs[1]);
    flow->state |= FC_FLOW_STATE_ABORT;
    return 0;
}

#pragma GCC diagnostic pop

f12r_call_t f12r_get_external_call(uint32_t num)
{
    f12r_call_t helper = NULL;
    switch(num) {
        case BPF_FUNC_BUILTIN_PRINTF:
            helper = &f12r_vm_printf;
            break;
        case BPF_FUNC_BUILTIN_ABORT:
            helper = &f12r_vm_abort;
            break;
        case BPF_FUNC_BUILTIN_ABORT_WITH_MSG:
            helper = &f12r_vm_abort_with_msg;
            break;
        default:
            break;
    }
#ifdef FEMTO_CONTAINER_USER_HELPER
    if (helper == NULL) {
        helper = femto_container_user_helper(num);
    }
#endif
    return helper;
}
