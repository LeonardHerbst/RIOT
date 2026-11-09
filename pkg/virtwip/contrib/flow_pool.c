/*
 * SPDX-FileCopyrightText: 2026 Leonard Herbst
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 *
 * @file
 * @brief   Flow slot pool implementation
 *
 * @author  Leonard Herbst <leonard.herbst@tu-dresden.de>
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>

#include "kernel_defines.h"
#include "mutex.h"

#include "femtocontainer/femtocontainer.h"
#include "flow_pool.h"

typedef struct {
    f12r_flow_t flow;
    f12r_mem_region_t scratch;
    uint8_t stack[VIRTWIP_FLOW_STACKSIZE] __attribute__((aligned(8)));
    bool used;
} virtwip_flow_slot_t;

static virtwip_flow_slot_t _slots[VIRTWIP_NUM_FLOWS];
static mutex_t _pool_lock = MUTEX_INIT;
static f12r_t *_vm;

void virtwip_flow_pool_init(f12r_t *vm)
{
    _vm = vm;
    for (unsigned i = 0; i < VIRTWIP_NUM_FLOWS; i++) {
        f12r_add_region(vm, &_slots[i].scratch, NULL, 0,
                        FC_MEM_REGION_READ | FC_MEM_REGION_WRITE);
    }
}

void virtwip_flow_pool_disable(void)
{
    mutex_lock(&_pool_lock);
    _vm = NULL;
    mutex_unlock(&_pool_lock);
}

int virtwip_flow_acquire(f12r_flow_t **flow)
{
    if (_vm == NULL) {
        /* VM not started or failed verification */
        return -ENODEV;
    }

    mutex_lock(&_pool_lock);
    for (unsigned i = 0; i < VIRTWIP_NUM_FLOWS; i++) {
        if (!_slots[i].used) {
            _slots[i].used = true;
            mutex_unlock(&_pool_lock);

            f12r_flow_t *f = &_slots[i].flow;
            *f = (f12r_flow_t){
                .f12r = _vm,
                .stack = _slots[i].stack,
                .stack_size = sizeof(_slots[i].stack),
            };
            *flow = f;
            return 0;
        }
    }
    mutex_unlock(&_pool_lock);
    return -ENOMEM;
}

void virtwip_flow_release(f12r_flow_t *flow)
{
    virtwip_flow_slot_t *slot = container_of(flow, virtwip_flow_slot_t, flow);

    mutex_lock(&_pool_lock);
    slot->used = false;
    mutex_unlock(&_pool_lock);
}

static int _flow_call(const char *func, void *args, size_t args_size,
                      const void *buf, size_t buf_len, int64_t *res)
{
    f12r_flow_t *flow;
    int err = virtwip_flow_acquire(&flow);

    if (err != 0) {
        return err;
    }

    virtwip_flow_slot_t *slot = container_of(flow, virtwip_flow_slot_t, flow);
    if (buf != NULL) {
        slot->scratch.start = buf;
        slot->scratch.len = buf_len;
    }

    int64_t r = 0;
    int fc = f12r_execute(flow, args, args_size, &r, func);

    if (buf != NULL) {
        slot->scratch.start = NULL;
        slot->scratch.len = 0;
    }

    virtwip_flow_release(flow);

    if (fc != FC_OK) {
        return -EIO;
    }
    *res = r;
    return 0;
}

int virtwip_flow_call(const char *func, void *args, size_t args_size, int64_t *res)
{
    return _flow_call(func, args, args_size, NULL, 0, res);
}

int virtwip_flow_call_buf(const char *func, void *args, size_t args_size,
                          const void *buf, size_t buf_len, int64_t *res)
{
    return _flow_call(func, args, args_size, buf, buf_len, res);
}

/** @} */
