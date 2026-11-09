/*
 * SPDX-FileCopyrightText: 2026 Leonard Herbst
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 *
 * @file
 * @brief   Pool of f12r flow slots for host threads executing VM code
 *
 * One flow is one concurrent VM execution context. A slot is held for the
 * whole duration of a VM execution (which may block inside a helper); a
 * thread may hold several slots at once for nested VM calls.
 */

#ifndef FLOW_POOL_H
#define FLOW_POOL_H

#include "femtocontainer/femtocontainer.h"
#include "sched.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef VIRTWIP_NUM_FLOWS
/**
 * @brief   Number of general flow slots, usable by every thread
 *
 * Bounds the number of host threads (sock API users, bootstrap) executing
 * VM code at the same time. A thread only holds a slot while executing VM
 * code; long waits (e.g. sock_udp_recv) happen outside the VM.
 */
#define VIRTWIP_NUM_FLOWS   (4)
#endif

#ifndef VIRTWIP_NUM_FLOWS_RX
/**
 * @brief   Slots reserved for the netdev RX thread
 *
 * Guarantees that received packets can always enter the VM, even if all
 * general slots are in use (see @ref virtwip_flow_pool_set_rx_thread).
 */
#define VIRTWIP_NUM_FLOWS_RX    (1)
#endif

#ifndef VIRTWIP_NUM_FLOWS_NESTED
/**
 * @brief   Slots reserved for nested VM calls
 *
 * A thread that already holds a slot (VM -> host helper -> VM again, e.g.
 * netif_add() -> lwip_netdev_init() -> VM) may use these, so a nested call
 * cannot fail because other threads took all general slots.
 */
#define VIRTWIP_NUM_FLOWS_NESTED    (1)
#endif

#ifndef VIRTWIP_FLOW_STACKSIZE
/**
 * @brief   BPF stack size per flow slot, in bytes
 *
 * Must cover the deepest VM call chain of a flow entry point plus
 * interpreter overhead.
 */
#define VIRTWIP_FLOW_STACKSIZE  (2048)
#endif

/**
 * @brief   Bind the pool to a VM
 *
 * @param[in] vm    VM the acquired flows execute on
 */
void virtwip_flow_pool_init(f12r_t *vm);

/**
 * @brief   Register the netdev RX thread
 *
 * Flow acquisitions from this thread may use the
 * @ref VIRTWIP_NUM_FLOWS_RX reserved slots.
 *
 * @param[in] pid   pid of the RX thread
 */
void virtwip_flow_pool_set_rx_thread(kernel_pid_t pid);

/**
 * @brief   Unbind the pool from its VM
 *
 * All following flow acquisitions fail with -ENODEV.
 */
void virtwip_flow_pool_disable(void);

/**
 * @brief   Acquire a flow slot from the pool
 *
 * @param[out] flow    initialized for the VM given to
 *                     @ref virtwip_flow_pool_init
 *
 * @return 0 on success
 * @return -ENODEV if the VM is not running
 * @return -ENOMEM if no slot is free
 *
 * Slot selection: the RX thread (@ref virtwip_flow_pool_set_rx_thread)
 * first tries its reserved slots, a thread already holding a slot first
 * tries the nested slots, then both fall back to the general slots.
 */
int virtwip_flow_acquire(f12r_flow_t **flow);

/**
 * @brief   Return a flow slot to the pool
 *
 * @param[in] flow    a flow previously acquired with @ref virtwip_flow_acquire
 */
void virtwip_flow_release(f12r_flow_t *flow);

/**
 * @brief   Run a VM trampoline function on a pooled flow
 *
 * @param[in] func        VM symbol to execute
 * @param[in] args        argument bundle (readable/writable by the VM), may
 *                        be NULL
 * @param[in] args_size   size of @p args
 * @param[out] res        return value of the VM function
 *
 * @return 0 on success
 * @return -ENODEV if the VM is not running
 * @return -ENOMEM if no slot is free
 * @return -EIO if the VM execution failed
 */
int virtwip_flow_call(const char *func, void *args, size_t args_size, int64_t *res);

/**
 * @brief   Run a VM trampoline that accesses a host buffer
 *
 * Same as @ref virtwip_flow_call, but the VM gets read/write access to
 * [@p buf, @p buf_len) for the duration of the call.
 */
int virtwip_flow_call_buf(const char *func, void *args, size_t args_size,
                          const void *buf, size_t buf_len, int64_t *res);

#ifdef __cplusplus
}
#endif

#endif /* FLOW_POOL_H */
/** @} */
