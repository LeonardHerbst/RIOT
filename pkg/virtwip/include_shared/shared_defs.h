#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SEM_T_SIZE (16)
typedef struct { uint8_t padding[SEM_T_SIZE]; } sys_sem_t;

#define SYS_MUTEX_T_SIIZE (8)
typedef struct { uint8_t padding[SYS_MUTEX_T_SIIZE]; } sys_mutex_t;

#define MBOX_T_SIZE   (40)
#define MSG_T_SIZE    (16)
#define SYS_MBOX_SIZE (8)
#define SYS_MBOX_T_SIZE (MBOX_T_SIZE + (MSG_T_SIZE * SYS_MBOX_SIZE))
typedef struct { uint8_t padding[SYS_MBOX_T_SIZE];  } sys_mbox_t;

typedef int16_t sys_thread_t;      /**< Thread typed to  */

#ifdef __cplusplus
}
#endif
