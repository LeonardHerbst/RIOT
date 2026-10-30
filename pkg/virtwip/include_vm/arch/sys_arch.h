#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* prefer mutexes rather than binary semaphores */
#define LWIP_COMPAT_MUTEX   (0)

#define SYS_ARCH_PROTECT(x)         sys_mutex_lock(&(x))
#define SYS_ARCH_UNPROTECT(x)       sys_mutex_unlock(&(x))
#define SYS_ARCH_DECL_PROTECT(x)    sys_mutex_t x; sys_mutex_new(&(x))

#define SEM_T_SIZE (16)
typedef struct { uint8_t padding[SEM_T_SIZE]; } sys_sem_t;

static inline bool sys_sem_valid(sys_sem_t *sem)
{
    return sem != NULL;
}

#define sys_sem_valid(sem)       (sys_sem_valid(sem))

#define sys_sem_set_invalid(sem)

#define SYS_MUTEX_T_SIIZE (8)
typedef struct { uint8_t padding[SYS_MUTEX_T_SIIZE]; } sys_mutex_t;

static inline bool sys_mutex_valid(sys_mutex_t *mutex)
{
    return mutex != NULL;
}

#define sys_mutex_valid(mutex)          (sys_mutex_valid(mutex))
#define sys_mutex_set_invalid(mutex)

#define MBOX_T_SIZE   (40)
#define MSG_T_SIZE    (16)
#define SYS_MBOX_SIZE (8)
#define SYS_MBOX_T_SIZE (MBOX_T_SIZE + (MSG_T_SIZE * SYS_MBOX_SIZE))

typedef struct { uint8_t padding[SYS_MBOX_T_SIZE];  } sys_mbox_t;

bool sys_mbox_valid(sys_mbox_t *mbox);
void sys_mbox_set_invalid(sys_mbox_t *mbox);

typedef int16_t sys_thread_t;      /**< Thread typed to  */

#define sys_mbox_valid(mbox)            (sys_mbox_valid(mbox))
#define sys_mbox_set_invalid(mbox)      (sys_mbox_set_invalid(mbox))

void sys_lock_tcpip_core(void);
#define LOCK_TCPIP_CORE()          sys_lock_tcpip_core()
void sys_unlock_tcpip_core(void);
#define UNLOCK_TCPIP_CORE()        sys_unlock_tcpip_core()

#ifdef MODULE_RANDOM
#define LWIP_RAND()                     (random_uint32())
#endif

#ifdef __cplusplus
}
#endif
