/*
 * SPDX-FileCopyrightText: 2026 Leonard Herbst
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 *
 * @file
 * @brief   Shared host glue shims around VM trampoline calls
 */

#ifndef VIRTWIP_GLUE_H
#define VIRTWIP_GLUE_H

#include "lwip/api.h"
#include "lwip/netbuf.h"
#include "lwip/pbuf.h"

#include "mbox.h"
#include "flow_pool.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Returned by virtwip_mbox_fetch() if the wait timed out */
#define VIRTWIP_MBOX_TIMEOUT    UINT32_MAX

/**
 * @brief   Waits on the host for a message in a VM-owned sys_mbox_t
 *
 * Same semantics as lwIP's sys_arch_mbox_fetch() (the VM's MBOX_FETCH helper
 * uses it too), but callable from host glue so that waiting does not occupy
 * a VM flow.
 *
 * @param[in]  mbox     mbox inside VM memory
 * @param[out] msg      fetched message
 * @param[in]  timeout  in ms, 0 waits forever
 *
 * @return  time waited in ms, or VIRTWIP_MBOX_TIMEOUT
 */
uint32_t virtwip_mbox_fetch(sys_mbox_t *mbox, void **msg, uint32_t timeout);

/* The VM's padded sys_mbox_t starts with the RIOT mbox (see femtoh_mbox_new) */
static inline mbox_t *virtwip_vm_conn_mbox(sys_mbox_t *mbox)
{
    return (mbox_t *)mbox;
}

static inline void virtwip_vm_pbuf_free(struct pbuf *p)
{
    virtwip_pbuf_p_args_t args = { .p = (bpfptr_t)(uintptr_t)p };
    int64_t res;
    virtwip_flow_call("virtwip_pbuf_free", &args, sizeof(args), &res);
}

static inline void virtwip_vm_conn_delete(struct netconn *conn)
{
    virtwip_netconn_args_t args = { .conn = (bpfptr_t)(uintptr_t)conn };
    int64_t res;
    virtwip_flow_call("virtwip_netconn_delete", &args, sizeof(args), &res);
}

static inline void virtwip_vm_netbuf_delete(struct netbuf *buf)
{
    virtwip_netbuf_args_t args = { .buf = (bpfptr_t)(uintptr_t)buf };
    int64_t res;
    virtwip_flow_call("virtwip_netbuf_delete", &args, sizeof(args), &res);
}

#if LWIP_TCP
static inline err_t virtwip_vm_conn_close(struct netconn *conn)
{
    virtwip_netconn_args_t args = { .conn = (bpfptr_t)(uintptr_t)conn };
    int64_t res;
    if (virtwip_flow_call("virtwip_netconn_close", &args, sizeof(args), &res) != 0) {
        return ERR_MEM;
    }
    return (err_t)res;
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* VIRTWIP_GLUE_H */
/** @} */
