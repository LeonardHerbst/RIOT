#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque blobs the host overlays with RIOT's sema_t, mutex_t and mbox_t:
 * 8-byte aligned so the overlaid RIOT types (which contain pointers) are
 * aligned on every host, and so the blobs sit at the same offsets on both
 * sides. */
#define SEM_T_SIZE (16)
typedef struct __attribute__((aligned(8))) { uint8_t padding[SEM_T_SIZE]; } sys_sem_t;

#define SYS_MUTEX_T_SIIZE (8)
typedef struct __attribute__((aligned(8))) { uint8_t padding[SYS_MUTEX_T_SIIZE]; } sys_mutex_t;

#define MBOX_T_SIZE   (40)
#define MSG_T_SIZE    (16)
#define SYS_MBOX_SIZE (8)
#define SYS_MBOX_T_SIZE (MBOX_T_SIZE + (MSG_T_SIZE * SYS_MBOX_SIZE))
typedef struct __attribute__((aligned(8))) { uint8_t padding[SYS_MBOX_T_SIZE];  } sys_mbox_t;

typedef int16_t sys_thread_t;      /**< Thread typed to  */

#ifdef __cplusplus
}
#endif
