/*
 * Copyright (C) 2020 Inria
 * Copyright (C) 2020 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef BPF_SHARED_H
#define BPF_SHARED_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define __bpf_shared_ptr(type, name) \
__attribute__((aligned(8)))          \
union {                              \
    type name;                       \
    uint64_t :64;                    \
}

enum {
    BPF_FUNC_BUILTIN_ABORT          = 0x100001,
    BPF_FUNC_BUILTIN_ABORT_WITH_MSG = 0x100002,
    BPF_FUNC_BUILTIN_PRINTF         = 0x100003,
    BPF_FUNC_BUILTIN_SPRINTF        = 0x100004,
    BPF_FUNC_BUILTIN_ATIO           = 0x100005,
};

#ifdef __cplusplus
}
#endif
#endif /* BPF_SHARED_H */
