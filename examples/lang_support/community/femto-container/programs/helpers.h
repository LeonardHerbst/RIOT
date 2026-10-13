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
#include "shared.h"

#ifdef __cplusplus
extern "C" {
#endif

static uint32_t (*bpf_user_test_helper)(void) = (void *)BPF_FUNC_USER_TEST_HELPER;

static uint32_t (*bpf_user_fp_dispatch)(call_ctx_t *) = (void *)BPF_FUNC_USER_FP_DISPATCH;

#ifdef __cplusplus
}
#endif
