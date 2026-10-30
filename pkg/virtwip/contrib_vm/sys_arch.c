/*
 * SPDX-FileCopyrightText: 2015 Martine Lenders <mlenders@inf.fu-berlin.de>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 *
 * @file
 */

#include <stdbool.h>
#include <stdint.h>

#include "arch/cc.h"
#include "arch/sys_arch.h"
#include "lwip/err.h"
#include "lwip/mem.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/tcpip.h"

#include "helpers.h"
#include "virtwip_opts.h"

void sys_init(void)
{
    return;
}

u32_t sys_now(void)
{
    return bpf_now_ms();
}

err_t sys_mutex_new(sys_mutex_t *mutex)
{
    bpf_mutex_new(mutex, SYS_MUTEX_T_SIIZE);
    return ERR_OK;
}

void sys_mutex_lock(sys_mutex_t *mutex)
{
    bpf_mutex_lock(mutex);
}

void sys_mutex_unlock(sys_mutex_t *mutex)
{
    bpf_mutex_unlock(mutex);
}

void sys_mutex_free(sys_mutex_t *mutex)
{
    (void) mutex;
    return;
}

err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
    bpf_sem_new(sem, count);
    return ERR_OK;
}

void sys_sem_free(sys_sem_t *sem)
{
    bpf_sem_free(sem);
}

void sys_sem_signal(sys_sem_t *sem)
{
    LWIP_ASSERT("invalid semaphore", sys_sem_valid(sem));
    bpf_sem_signal(sem);
}

u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t count)
{
    LWIP_ASSERT("invalid semaphore", sys_sem_valid(sem));
    if (bpf_sem_wait_ms(sem, count) == UINT32_MAX) {
        return SYS_ARCH_TIMEOUT;
    }
    return 0;
}

err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
    bpf_mbox_new(mbox, size);
    return ERR_OK;
}

void sys_mbox_free(sys_mbox_t *mbox)
{
    return;
}

void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
    if (bpf_mbox_post(mbox, msg) != 0) {
        LWIP_ASSERT("invalid mbox", false);
    }
}

err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
    uint32_t res = bpf_mbox_trypost(mbox, msg);
    LWIP_ASSERT("invalid mbox", res != UINT32_MAX);
    if (res == 1) {
        return ERR_OK;
    }
    else {
        return ERR_MEM;
    }
}

err_t sys_mbox_trypost_fromisr(sys_mbox_t *mbox, void *msg)
{
    LWIP_ASSERT("Unimplemented!", false);
}

u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
    int res = bpf_mbox_fetch(mbox, msg, timeout);
    if (res == UINT32_MAX) {
        return SYS_ARCH_TIMEOUT;
    }
    return res;
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
    if (bpf_mbox_tryfetch(mbox, msg)) {
        return ERR_OK;
    } else {
        return SYS_MBOX_EMPTY;
    }
}

bool sys_mbox_valid(sys_mbox_t *mbox)
{
    return (mbox != NULL) && bpf_mbox_valid(mbox);
}

void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
    if (mbox != NULL) {
        bpf_mbox_set_invalid(mbox);
    }
}

sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg,
                            int stacksize, int prio)
{
    char *bpf_stack = mem_malloc((size_t)stacksize);
    char *os_stack = mem_malloc(VIRTWIP_TCPIP_OS_STACKSIZE);
    if (bpf_stack == NULL || os_stack == NULL) {
        return ERR_MEM;
    }

    virtwip_thread_info_t params = {
        .bpf_stack = bpf_stack,
        .bpf_stack_size = (u16_t) stacksize,
        .os_stack = os_stack,
        .os_stack_size = VIRTWIP_TCPIP_OS_STACKSIZE,
        .prio = prio,
        /* this assumes the lwip_thread_fn was typedefed to char* */
        .vm_func = (char *) thread,
        .arg = arg,
        .name = name
    };

    s16_t res = bpf_thread_new(&params);
    if (res <= 0) {
        bpf_builtin_abort_with_msg("Thread creation failed!\n");
    }

    return res;
}

s16_t lwip_tcpip_thread = 0;
static s16_t lwip_lock_thread;

void sys_mark_tcpip_thread(void) {
    lwip_tcpip_thread = bpf_thread_getpid();
}

void sys_lock_tcpip_core(void) {
    sys_mutex_lock(&lock_tcpip_core);
    lwip_lock_thread = bpf_thread_getpid();
}

void sys_unlock_tcpip_core(void) {
    /* 0 is KERNEL_PID_UNDEF from sched.h */
    lwip_lock_thread = 0;
    sys_mutex_unlock(&lock_tcpip_core);
}

bool sys_check_core_locked(void) {
    /* This is called from inside the VM which can not be called from an ISR context */
    if (lwip_tcpip_thread != 0) {
        /* only call from thread with lock */
        return lwip_lock_thread == bpf_thread_getpid();
    }
    return true;
}

/** @} */
