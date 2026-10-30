#pragma once

#include "builtin_shared.h"

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
    BPF_FUNC_CALL_NETCONN_CB,
    BPF_FUNC_CALL_NETIF_INIT_FN,
    BPF_FUNC_CALL_NETIF_LINKOUTPUT_FN,
    BPF_FUNC_CALL_SLIP_OUTPUT4,
    BPF_FUNC_CALL_SLIP_OUTPUT6,
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

#define _MSG_SUCCESS    (0x5cac)
#define _MSG_TIMEOUT    (0x5cad)

