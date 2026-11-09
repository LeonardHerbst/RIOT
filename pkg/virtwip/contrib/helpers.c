// #ifdef FEMTO_CONTAINER_USER_HELPER
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include "mbox.h"
#include "mutex.h"
#include "sched.h"
#include "sema.h"
#include "shared.h"
#include "thread.h"
#include "ztimer.h"

#include "net/netif.h"
#include "sock_types.h"
#include "lwip_init_devs.h"

#include "femtocontainer/femtocontainer.h"
#include "lwip/api.h"
#include "lwip/netif.h"

#define ARG1 (uintptr_t)regs[1]
#define ARG2 (uintptr_t)regs[2]
#define ARG3 (uintptr_t)regs[3]
#define ARG4 (uintptr_t)regs[4]

/* Ztimer Helper */
static uint32_t femtoh_now_ms(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    (void)regs;
    return ztimer_now(ZTIMER_MSEC);
}

/* Mutex Helper */
static uint32_t femtoh_mutex_new(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    mutex_t *mutex = (mutex_t *)ARG1;
    mutex_init(mutex);
    return 0;
}

static uint32_t femtoh_mutex_lock(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    mutex_t *mutex = (mutex_t *)ARG1;
    mutex_lock(mutex);
    return 0;
}

static uint32_t femtoh_mutex_unlock(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    mutex_t *mutex = (mutex_t *)ARG1;
    mutex_unlock(mutex);
    return 0;
}

/* Semaphore Helper */
static uint32_t femtoh_sem_new(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    sema_t *sem = (sema_t *)ARG1;
    unsigned int count = (unsigned int)ARG2;

    sema_create(sem, count);
    return 0;
}

static uint32_t femtoh_sem_free(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    sema_t *sem = (sema_t *)ARG1;

    sema_destroy(sem);
    return 0;
}

static uint32_t femtoh_sem_signal(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    sema_t *sem = (sema_t *)ARG1;

    sema_post(sem);
    return 0;
}

static uint32_t femtoh_sem_wait_ms(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    sema_t *sem = (sema_t *)ARG1;
    unsigned int count = (unsigned int)ARG2;

    if (count != 0) {
        uint32_t start = ztimer_now(ZTIMER_MSEC);
        int res = sema_wait_timed_ztimer(sem, ZTIMER_MSEC, count);
        uint32_t stop = ztimer_now(ZTIMER_MSEC);
        if (res == -ETIMEDOUT) {
            return UINT32_MAX;
        }
        return stop - start;
    }
    else {
        sema_wait((sema_t *)sem);
        return 0;
    }
}

/* Mbox Helper */
typedef struct {
    mbox_t mbox;               /**< RIOT mbox */
    msg_t msgs[SYS_MBOX_SIZE]; /**< queue for the mbox */
} virtwip_mbox_t;

static uint32_t femtoh_mbox_new(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    virtwip_mbox_t *mbox = (virtwip_mbox_t *)ARG1;
    mbox_init(&mbox->mbox, mbox->msgs, SYS_MBOX_SIZE);

    return 0;
}

static uint32_t femtoh_mbox_free(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    (void)regs;
    /* RIOT mboxes hold no resources that need to be released */
    return 0;
}

static uint32_t femtoh_mbox_post(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    virtwip_mbox_t *mbox = (virtwip_mbox_t *)ARG1;
    void *msg = (void *)ARG2;

    if (mbox == NULL || mbox_size(&mbox->mbox) == 0) {
        return UINT32_MAX;
    }

    msg_t m = { .content = { .ptr = msg }, .type = _MSG_SUCCESS };
    mbox_put(&mbox->mbox, &m);

    return 0;
}

static uint32_t femtoh_mbox_trypost(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    virtwip_mbox_t *mbox = (virtwip_mbox_t *)ARG1;
    void *msg = (void *)ARG2;

    if (mbox == NULL || mbox_size(&mbox->mbox) == 0) {
        return UINT32_MAX;
    }

    msg_t m = { .content = { .ptr = msg }, .type = _MSG_SUCCESS };
    return (uint32_t)mbox_try_put(&mbox->mbox, &m);
}

static void _mbox_timeout(void *arg)
{
    msg_t m = { .type = _MSG_TIMEOUT };
    mbox_put(arg, &m);
}

static uint32_t femtoh_mbox_fetch(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    virtwip_mbox_t *mbox = (virtwip_mbox_t *)ARG1;
    void **msg = (void **)ARG2;
    uint32_t timeout = (uint32_t)ARG3;

    msg_t m;
    ztimer_t timer = { .callback = _mbox_timeout, .arg = &mbox->mbox };

    uint32_t start = ztimer_now(ZTIMER_MSEC);
    if (timeout > 0) {
        ztimer_set(ZTIMER_MSEC, &timer, timeout);
    }
    mbox_get(&mbox->mbox, &m);
    uint32_t stop = ztimer_now(ZTIMER_MSEC);
    ztimer_remove(ZTIMER_MSEC, &timer); /* in case timer did not time out */
    switch (m.type) {
    case _MSG_SUCCESS:
        *(bpfptr_t *)msg = (bpfptr_t)(uintptr_t)m.content.ptr;
        return stop - start;
    case _MSG_TIMEOUT:
        break;
    default: /* should not happen */
        printf("Virtwip: Invalid message received\n");
        assert(false);
        break;
    }
    return UINT32_MAX;
}

static uint32_t femtoh_mbox_tryfetch(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    virtwip_mbox_t *mbox = (virtwip_mbox_t *)ARG1;
    void **msg = (void **)ARG2;

    msg_t m;

    if (mbox_try_get(&mbox->mbox, &m)) {
        assert(m.type == _MSG_SUCCESS);
        *(bpfptr_t *)msg = (bpfptr_t)(uintptr_t)m.content.ptr;
        return 1;
    }
    return 0;
}

static uint32_t femtoh_mbox_valid(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    virtwip_mbox_t *mbox = (virtwip_mbox_t *)ARG1;

    return (uint32_t)(mbox_size(&mbox->mbox) != 0);
}

static uint32_t femtoh_mbox_set_invalid(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    virtwip_mbox_t *mbox = (virtwip_mbox_t *)ARG1;

    mbox_unset(&mbox->mbox);
    return 0;
}

/* Thread Helper */
/* Support structure for creating new threads */
typedef struct {
    mutex_t sync;
    f12r_flow_t *flow;
    char *bpf_stack;
    int bpf_stack_size;
    char *vm_func;
    void *arg;
} _thread_starter_params_t;

static void *_thread_starter(void *params_ptr)
{
    _thread_starter_params_t *params = params_ptr;
    char *vm_func = params->vm_func;
    void *arg = params->arg;

    f12r_flow_t flow = {
        .f12r = params->flow->f12r,
        .stack = params->bpf_stack,
        .stack_size = params->bpf_stack_size,
    };

    mutex_unlock(&params->sync);

    int64_t result;
    f12r_execute(&flow, arg, sizeof(intptr_t), &result, vm_func);

    return NULL;
}

static uint32_t femtoh_thread_new(f12r_flow_t *flow, uint64_t *regs)
{
    virtwip_thread_info_t *params = (virtwip_thread_info_t *)ARG1;

    _thread_starter_params_t thread_params = {
        .sync = MUTEX_INIT_LOCKED,
        .bpf_stack = params->bpf_stack,
        .bpf_stack_size = params->bpf_stack_size,
        .flow = flow,
        .vm_func = params->vm_func,
        .arg = (void *)(intptr_t)params->arg,
    };

    kernel_pid_t res = thread_create(params->os_stack, params->os_stack_size, params->prio, 0,
                                     &_thread_starter, (void *)&thread_params,
                                     (char *)params->name);
    if (res <= 0) {
        return (uint32_t)res;
    }

    mutex_lock(&thread_params.sync);

    thread_yield_higher();
    return (uint32_t)res;
}

static uint32_t femtoh_thread_getpid(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    (void)regs;
    return thread_getpid();
}

static uint32_t femtoh_call_netconn_cb(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    struct netconn *conn = (struct netconn *)ARG1;
    enum netconn_evt evt = (enum netconn_evt)ARG2;
    int len = (int)ARG3;
    netconn_cb(conn, evt, len);
    return 0;
}

static uint32_t femtoh_call_fp_netif_init_fn(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    netif_init_fn_raw fn = (netif_init_fn_raw)ARG1;
    netif_t *netif = (netif_t *)ARG2;

    // TODO check against a whitelist to which legal callbacks are added during
    // initialization
    return (uint32_t)fn(netif);
}

static uint32_t femtoh_call_fp_netif_linkoutput_fn(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    netif_linkoutput_fn_raw fn = (netif_linkoutput_fn_raw)ARG1;
    netif_t *netif = (netif_t *)ARG2;
    pbuf_t *p = (pbuf_t *)ARG3;

    // TODO check against a whitelist to which legal callbacks are added during
    // initialization
    return (uint32_t)fn(netif, p);
}

#if MODULE_SLIPDEV_NET && LWIP_IPV4
static uint32_t femtoh_call_slip_output4(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    netif_t *netif = (netif_t *)ARG1;
    pbuf_t *p = (pbuf_t *)ARG2;
    const ip4_addr_t *addr = (const ip4_addr_t *)ARG3;

    return (uint32_t)slip_output4(netif, p, addr);
}
#endif /* MODULE_SLIPDEV_NET && LWIP_IPV4 */

#if MODULE_SLIPDEV_NET && LWIP_IPV6
static uint32_t femtoh_call_slip_output6(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    netif_t *netif = (netif_t *)ARG1;
    pbuf_t *p = (pbuf_t *)ARG2;
    const ip6_addr_t *addr = (const ip6_addr_t *)ARG3;

    return (uint32_t)slip_output6(netif, p, addr);
}
#endif /* MODULE_SLIPDEV_NET && LWIP_IPV6 */

f12r_call_t femto_container_user_helper(uint32_t num)
{
    switch (num) {
    case BPF_FUNC_NOW_MS:
        return &femtoh_now_ms;
    case BPF_FUNC_MUTEX_NEW:
        return &femtoh_mutex_new;
    case BPF_FUNC_MUTEX_LOCK:
        return &femtoh_mutex_lock;
    case BPF_FUNC_MUTEX_UNLOCK:
        return &femtoh_mutex_unlock;
    case BPF_FUNC_SEM_NEW:
        return &femtoh_sem_new;
    case BPF_FUNC_SEM_FREE:
        return &femtoh_sem_free;
    case BPF_FUNC_SEM_SIGNAL:
        return &femtoh_sem_signal;
    case BPF_FUNC_SEM_WAIT_MS:
        return &femtoh_sem_wait_ms;
    case BPF_FUNC_MBOX_NEW:
        return &femtoh_mbox_new;
    case BPF_FUNC_MBOX_FREE:
        return &femtoh_mbox_free;
    case BPF_FUNC_MBOX_POST:
        return &femtoh_mbox_post;
    case BPF_FUNC_MBOX_TRYPOST:
        return &femtoh_mbox_trypost;
    case BPF_FUNC_MBOX_FETCH:
        return &femtoh_mbox_fetch;
    case BPF_FUNC_MBOX_TRYFETCH:
        return &femtoh_mbox_tryfetch;
    case BPF_FUNC_MBOX_VALID:
        return &femtoh_mbox_valid;
    case BPF_FUNC_MBOX_SET_INVALID:
        return &femtoh_mbox_set_invalid;
    case BPF_FUNC_THREAD_NEW:
        return &femtoh_thread_new;
    case BPF_FUNC_THREAD_GETPID:
        return &femtoh_thread_getpid;
    case BPF_FUNC_CALL_NETCONN_CB:
        return &femtoh_call_netconn_cb;
#if MODULE_SLIPDEV_NET && LWIP_IPV4
    case BPF_FUNC_CALL_SLIP_OUTPUT4:
        return &femtoh_call_slip_output4;
#endif /* MODULE_SLIPDEV_NET && LWIP_IPV4 */
#if MODULE_SLIPDEV_NET && LWIP_IPV6
    case BPF_FUNC_CALL_SLIP_OUTPUT6:
        return &femtoh_call_slip_output6;
#endif /* MODULE_SLIPDEV_NET && LWIP_IPV6 */
    case BPF_FUNC_CALL_FP_NETIF_INIT_FN:
        return &femtoh_call_fp_netif_init_fn;
    case BPF_FUNC_CALL_FP_NETIF_LINKOUTPUT_FN:
        return &femtoh_call_fp_netif_linkoutput_fn;
    default:
        return NULL;
    }
}
// #endif
