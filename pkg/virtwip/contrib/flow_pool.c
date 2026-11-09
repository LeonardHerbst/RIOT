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
#include "thread.h"

#include "femtocontainer/femtocontainer.h"
#include "flow_pool.h"

typedef struct {
    f12r_flow_t flow;
    uint8_t stack[VIRTWIP_FLOW_STACKSIZE] __attribute__((aligned(8)));
    bool used;
    kernel_pid_t owner;     /**< thread holding the slot */
} virtwip_flow_slot_t;

/* Slot layout: [general | reserved RX | reserved nested] */
#define _GENERAL_FIRST  (0)
#define _RX_FIRST       (VIRTWIP_NUM_FLOWS)
#define _NESTED_FIRST   (_RX_FIRST + VIRTWIP_NUM_FLOWS_RX)
#define _NUM_SLOTS      (_NESTED_FIRST + VIRTWIP_NUM_FLOWS_NESTED)

static virtwip_flow_slot_t _slots[_NUM_SLOTS];
static mutex_t _pool_lock = MUTEX_INIT;
static f12r_t *_vm;
static kernel_pid_t _rx_thread = KERNEL_PID_UNDEF;

void virtwip_flow_pool_init(f12r_t *vm)
{
    mutex_lock(&_pool_lock);
    _vm = vm;
    mutex_unlock(&_pool_lock);
}

void virtwip_flow_pool_disable(void)
{
    mutex_lock(&_pool_lock);
    _vm = NULL;
    mutex_unlock(&_pool_lock);
}

void virtwip_flow_pool_set_rx_thread(kernel_pid_t pid)
{
    mutex_lock(&_pool_lock);
    _rx_thread = pid;
    mutex_unlock(&_pool_lock);
}

/* Returns the index of a free slot in [first, first + num), or -1 */
static int _find_free(unsigned first, unsigned num)
{
    for (unsigned i = first; i < first + num; i++) {
        if (!_slots[i].used) {
            return (int)i;
        }
    }
    return -1;
}

static bool _holds_slot(kernel_pid_t pid)
{
    for (unsigned i = 0; i < _NUM_SLOTS; i++) {
        if (_slots[i].used && (_slots[i].owner == pid)) {
            return true;
        }
    }
    return false;
}

int virtwip_flow_acquire(f12r_flow_t **flow)
{
    kernel_pid_t pid = thread_getpid();
    int idx = -1;

    mutex_lock(&_pool_lock);
    if (_vm == NULL) {
        /* VM not started or failed verification */
        mutex_unlock(&_pool_lock);
        return -ENODEV;
    }
    if (pid == _rx_thread) {
        idx = _find_free(_RX_FIRST, VIRTWIP_NUM_FLOWS_RX);
    }
    if ((idx < 0) && _holds_slot(pid)) {
        idx = _find_free(_NESTED_FIRST, VIRTWIP_NUM_FLOWS_NESTED);
    }
    if (idx < 0) {
        idx = _find_free(_GENERAL_FIRST, VIRTWIP_NUM_FLOWS);
    }
    if (idx < 0) {
        mutex_unlock(&_pool_lock);
        return -ENOMEM;
    }
    _slots[idx].used = true;
    _slots[idx].owner = pid;
    f12r_t *vm = _vm;
    mutex_unlock(&_pool_lock);

    f12r_flow_t *f = &_slots[idx].flow;
    *f = (f12r_flow_t){
        .f12r = vm,
        .stack = _slots[idx].stack,
        .stack_size = sizeof(_slots[idx].stack),
    };
    *flow = f;
    return 0;
}

void virtwip_flow_release(f12r_flow_t *flow)
{
    virtwip_flow_slot_t *slot = container_of(flow, virtwip_flow_slot_t, flow);

    mutex_lock(&_pool_lock);
    slot->used = false;
    slot->owner = KERNEL_PID_UNDEF;
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

    if (buf != NULL) {
        /* Only accessible by this flow; f12r_execute() clears it again */
        flow->scratch_region = (f12r_mem_region_t){
            .start = buf,
            .len = buf_len,
            .flag = FC_MEM_REGION_READ | FC_MEM_REGION_WRITE,
        };
    }

    int64_t r = 0;
    int fc = f12r_execute(flow, args, args_size, &r, func);

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
