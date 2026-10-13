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


enum {
    BPF_FUNC_USER_TEST_HELPER = 0xF0,
    BPF_FUNC_USER_FP_DISPATCH = 0xF1,
};

#ifdef __cplusplus
}
#endif
