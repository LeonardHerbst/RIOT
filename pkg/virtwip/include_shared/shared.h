#pragma once

#include "builtin_shared.h"

#include "lwip/ip_addr.h"

/* Two restrictions:
 *  - 0 is not a valid helper ID
 *  - 0x100000 and above are reserved for builin helpers */
enum {
    BPF_FUN_INVALID,
    BPF_FUNC_NOW_MS,
    BPF_FUNC_MUTEX_NEW,
    BPF_FUNC_MUTEX_LOCK,
    BPF_FUNC_MUTEX_UNLOCK,
    BPF_FUNC_SEM_NEW,
    BPF_FUNC_SEM_FREE,
    BPF_FUNC_SEM_SIGNAL,
    BPF_FUNC_SEM_WAIT_MS,
    BPF_FUNC_MBOX_NEW,
    BPF_FUNC_MBOX_FREE,
    BPF_FUNC_MBOX_POST,
    BPF_FUNC_MBOX_TRYPOST,
    BPF_FUNC_MBOX_FETCH,
    BPF_FUNC_MBOX_TRYFETCH,
    BPF_FUNC_MBOX_VALID,
    BPF_FUNC_MBOX_SET_INVALID,
    BPF_FUNC_THREAD_NEW,
    BPF_FUNC_THREAD_GETPID,
    /* This used to be a dynamic VM->host function call but the fp was always
     * set to the _netconn_cb function. Thus this becomes a regular helper. */
    BPF_FUNC_CALL_NETCONN_CB,
    /* The netif_output_fn is a VM->VM function with one VM->host option (this). */
    BPF_FUNC_CALL_SLIP_OUTPUT4,
    /* The netif_output_ip6_fn is a VM->VM function with one VM->host option (this). */
    BPF_FUNC_CALL_SLIP_OUTPUT6,
    /* These functions call a function pointer passed to
     * them (as a bpfptr_t aka uint64_t). This is used for indirect VM->host calls. */
    BPF_FUNC_CALL_FP_NETIF_INIT_FN,
    BPF_FUNC_CALL_FP_NETIF_LINKOUTPUT_FN,
};

typedef struct {
    __bpf_shared_ptr(char *, bpf_stack);
    uint16_t bpf_stack_size;
    __bpf_shared_ptr(char *, os_stack);
    uint16_t os_stack_size;
    uint16_t prio;
    __bpf_shared_ptr(char *, vm_func);
    __bpf_shared_ptr(void *, arg);
    __bpf_shared_ptr(const char *, name);
} virtwip_thread_info_t;

typedef struct {
    bpfptr_t p;     /* struct pbuf * */
    bpfptr_t netif; /* struct netif * */
} virtwip_input_args_t;

/* Argument bundles for host glue -> VM trampoline calls. Pointer fields are
 * bpfptr_t (8 bytes on both sides), scalars use fixed-width types so the
 * layouts match. ip_addr_t members are read/written through the call's arg
 * region. */

typedef struct {
    bpfptr_t netif;
    bpfptr_t state;
    bpfptr_t init;
    uint32_t input;
} virtwip_netif_add_noaddr_args_t;

typedef struct {
    bpfptr_t netif;
} virtwip_netif_args_t;

typedef struct {
    bpfptr_t done;
    bpfptr_t arg;
} virtwip_tcpip_init_args_t;

typedef struct {
    bpfptr_t ip6;
    bpfptr_t netif;
    uint32_t type;
} virtwip_ip6_addr_zone_args_t;

typedef struct {
    uint32_t layer;
    uint32_t len;
    uint32_t type;
} virtwip_pbuf_alloc_args_t;

typedef struct {
    bpfptr_t p;
} virtwip_pbuf_p_args_t;

typedef struct {
    bpfptr_t p;
    bpfptr_t buf;
    uint32_t len;
} virtwip_pbuf_take_args_t;

typedef struct {
    bpfptr_t p;
    bpfptr_t buf;
    uint32_t len;
    uint32_t offset;
} virtwip_pbuf_take_at_args_t;

typedef struct {
    bpfptr_t p;
    int32_t size;
} virtwip_pbuf_header_args_t;

typedef struct {
    bpfptr_t p;
    bpfptr_t buf;
    uint32_t len;
    uint32_t offset;
} virtwip_pbuf_copy_partial_args_t;

typedef struct {
    uint32_t pan_id;
} virtwip_lowpan6_pan_id_args_t;

typedef struct {
    bpfptr_t netif;
    uint32_t from_mac;
} virtwip_netif_ip6_linklocal_args_t;

typedef struct {
    uint32_t type;
    uint32_t proto;
    uint32_t callback;
} virtwip_netconn_new_args_t;

typedef struct {
    bpfptr_t conn;
    ip_addr_t addr;
    uint32_t port;
} virtwip_netconn_ep_args_t;

typedef struct {
    bpfptr_t conn;
} virtwip_netconn_args_t;

typedef struct {
    bpfptr_t conn;
    uint32_t local;
    ip_addr_t addr;     /* out */
    uint32_t port;      /* out */
} virtwip_netconn_getaddr_args_t;

typedef struct {
    bpfptr_t conn;
    bpfptr_t buf;       /* out */
} virtwip_netconn_recv_args_t;

/* Receiving is split so the host can wait for the recvmbox outside the VM:
 * prepare, (host waits if must_wait), then finish or abort (timeout). */
typedef struct {
    bpfptr_t conn;
    bpfptr_t msg;       /* out: message fetched without waiting */
    uint32_t must_wait; /* out: host has to wait on conn->recvmbox */
} virtwip_netconn_recv_prepare_args_t;

typedef struct {
    bpfptr_t conn;
    bpfptr_t msg;       /* message fetched from conn->recvmbox */
    bpfptr_t buf;       /* out: received netbuf */
} virtwip_netconn_recv_finish_args_t;

typedef struct {
    bpfptr_t buf;
    uint32_t size;
} virtwip_netbuf_alloc_args_t;

typedef struct {
    bpfptr_t buf;
} virtwip_netbuf_args_t;

typedef struct {
    bpfptr_t conn;
    bpfptr_t netbuf;
    ip_addr_t addr;
    uint32_t port;
} virtwip_netconn_sendto_args_t;

typedef struct {
    bpfptr_t conn;
    bpfptr_t netbuf;
} virtwip_netconn_send_args_t;

typedef struct {
    bpfptr_t conn;
    bpfptr_t dataptr;
    uint32_t len;
    uint32_t apiflags;
    uint32_t written;   /* out */
} virtwip_netconn_write_args_t;

typedef struct {
    bpfptr_t conn;
    uint32_t backlog;
} virtwip_netconn_listen_args_t;

typedef struct {
    ip_addr_t addr;
} virtwip_sock_bind_addr_args_t;

/* ip_type is an lwIP IPADDR_TYPE_V4/IPADDR_TYPE_V6 (the host maps its
 * AF_* family, RIOT's net/af.h is not available inside the VM) */
typedef struct {
    uint32_t ip_type;
    uint32_t netif_num;
} virtwip_sock_netif_num_args_t;

typedef struct {
    uint32_t ip_type;
    uint32_t netif_num;
    ip_addr_t addr;
} virtwip_sock_addr_on_netif_args_t;

#define _MSG_SUCCESS    (0x5cac)
#define _MSG_TIMEOUT    (0x5cad)

