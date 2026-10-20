/*
 * Copyright (C) 2020 Inria
 * Copyright (C) 2020 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define __bpf_shared_ptr(type, name) \
__attribute__((aligned(8)))          \
union {                              \
    type name;                       \
    uint64_t :64;                    \
}


typedef uint32_t (*user_helper_function_t)(void);

typedef struct {
    __bpf_shared_ptr(user_helper_function_t, fp);
} call_ctx_t;

#define SYS_MUTEX_T_SIIZE (8)

typedef struct { uint8_t padding[SYS_MUTEX_T_SIIZE]; } sys_mutex_t;

typedef struct {
    __bpf_shared_ptr(char *, bpf_stack);
    uint16_t bpf_stack_size;
    __bpf_shared_ptr(char *, os_stack);
    uint16_t os_stack_size;
    uint16_t prio;
    __bpf_shared_ptr(char *, vm_func);
    __bpf_shared_ptr(void *, arg);
    __bpf_shared_ptr(const char *, name);
} bpf_thread_info_t;

enum {
    BPF_FUNC_USER_TEST_HELPER = 0xF0,
    BPF_FUNC_USER_FP_DISPATCH = 0xF1,
    BPF_FUNC_MUTEX_NEW        = 0xF2,
    BPF_FUNC_MUTEX_LOCK       = 0xF3,
    BPF_FUNC_MUTEX_UNLOCK     = 0xF4,
    BPF_FUNC_WAIT_MS          = 0xF5,
    BPF_FUNC_THREAD_NEW       = 0xF6,
};

#ifdef __cplusplus
}
#endif
