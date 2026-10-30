#pragma once

#include "arch/sys_arch.h"
#include "lwip/arch.h"
#include "lwip/err.h"

#include "shared.h"

static u32_t (*bpf_now_ms)(void) = (void *) BPF_FUNC_NOW_MS;

static void (*bpf_mutex_new)(sys_mutex_t *mutex, u8_t size) = (void *) BPF_FUNC_MUTEX_NEW;
static void (*bpf_mutex_lock)(sys_mutex_t *mutex)           = (void *) BPF_FUNC_MUTEX_LOCK;
static void (*bpf_mutex_unlock)(sys_mutex_t *mutex)         = (void *) BPF_FUNC_MUTEX_UNLOCK;

static void (*bpf_sem_new)(sys_sem_t *sem, u8_t size)        = (void *) BPF_FUNC_SEM_NEW;
static void (*bpf_sem_free)(sys_sem_t *sem)                  = (void *) BPF_FUNC_SEM_FREE;
static void (*bpf_sem_signal)(sys_sem_t *sem)                = (void *) BPF_FUNC_SEM_SIGNAL;
static u32_t (*bpf_sem_wait_ms)(sys_sem_t *sem, u32_t count) = (void *) BPF_FUNC_SEM_WAIT_MS;

static void (*bpf_mbox_new)(sys_mbox_t *mbox, u8_t size)                    = (void *) BPF_FUNC_MBOX_NEW;
static u32_t (*bpf_mbox_post)(sys_mbox_t *mbox, void* msg)                  = (void *) BPF_FUNC_MBOX_POST;
static u32_t (*bpf_mbox_trypost)(sys_mbox_t *mbox, void* msg)               = (void *) BPF_FUNC_MBOX_TRYPOST;
static u32_t (*bpf_mbox_fetch)(sys_mbox_t *mbox, void **msg, u32_t timeout) = (void *) BPF_FUNC_MBOX_FETCH;
static u32_t (*bpf_mbox_tryfetch)(sys_mbox_t *mbox, void **msg)             = (void *) BPF_FUNC_MBOX_TRYFETCH;
static bool (*bpf_mbox_valid)(sys_mbox_t *mbox)                             = (void *) BPF_FUNC_MBOX_VALID;
static bool (*bpf_mbox_set_invalid)(sys_mbox_t *mbox)                       = (void *) BPF_FUNC_MBOX_SET_INVALID;

static s16_t (*bpf_thread_new)(virtwip_thread_info_t *params) = (void *) BPF_FUNC_THREAD_NEW;
static s16_t (*bpf_thread_getpid)()                           = (void *) BPF_FUNC_THREAD_GETPID;
