/*
 * Copyright (C) 2020 Inria
 * Copyright (C) 2020 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#pragma once

#include "shared.h"

#ifdef __cplusplus
extern "C" {
#endif

static uint32_t (*bpf_user_test_helper)(void) = (void *)BPF_FUNC_USER_TEST_HELPER;

static uint32_t (*bpf_user_fp_dispatch)(call_ctx_t *) = (void *)BPF_FUNC_USER_FP_DISPATCH;

static void (*bpf_mutex_new)(sys_mutex_t *mutex, uint8_t size) = (void *)BPF_FUNC_MUTEX_NEW;
static void (*bpf_mutex_lock)(sys_mutex_t *mutex)              = (void *)BPF_FUNC_MUTEX_LOCK;
static void (*bpf_mutex_unlock)(sys_mutex_t *mutex)            = (void *)BPF_FUNC_MUTEX_UNLOCK;

static void (*bpf_wait_ms)(uint32_t time) = (void *)BPF_FUNC_WAIT_MS;

static int16_t (*bpf_thread_new)(bpf_thread_info_t *params) = (void *)BPF_FUNC_THREAD_NEW;

#ifdef __cplusplus
}
#endif
