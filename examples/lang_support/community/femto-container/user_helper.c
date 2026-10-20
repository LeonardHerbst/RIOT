#include <stdint.h>
#ifdef FEMTO_CONTAINER_USER_HELPER
#  include <stdio.h>

#  include "femtocontainer/femtocontainer.h"
#  include "mutex.h"
#  include "programs/shared.h"
#  include "ztimer.h"

uint32_t user_test_helper(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    (void)regs;
    printf("User test helper called\n");
    return 42;
}

uint32_t user_fp_dispatch(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    call_ctx_t *ctx = (call_ctx_t *)(uintptr_t)regs[1];
    printf("Context struct size on the host: %ld\n", (long int)sizeof(*ctx));
    return ctx->fp();
}

uint32_t user_bpf_mutex_new(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    mutex_t *mutex = (mutex_t *)(uintptr_t)regs[1];
    uint8_t size = (uint8_t)(uintptr_t)regs[2];
    assert(sizeof(mutex_t) <= size);
    mutex_init(mutex);
    return 0;
}

uint32_t user_bpf_mutex_lock(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    mutex_t *mutex = (mutex_t *)(uintptr_t)regs[1];
    mutex_lock(mutex);
    return 0;
}

uint32_t user_bpf_mutex_unlock(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    mutex_t *mutex = (mutex_t *)(uintptr_t)regs[1];
    mutex_unlock(mutex);
    return 0;
}

uint32_t user_bpf_wait_ms(f12r_flow_t *flow, uint64_t *regs)
{
    (void)flow;
    uint32_t time = (uint32_t)(uintptr_t)regs[1];
    ztimer_sleep(ZTIMER_MSEC, time);
    return 0;
}

typedef struct {
    mutex_t sync;
    char *stack;
    int stack_size;
    f12r_flow_t *flow;
    char *vm_func;
    void *arg;
} _thread_starter_params_t;

static void *_thread_starter(void *params_ptr)
{
    _thread_starter_params_t *params = params_ptr;

    int64_t result;
    const char *fn_name = params->vm_func;
    void *arg = params->arg;

    f12r_flow_t flow = {
        .f12r = params->flow->f12r,
        .stack = (unsigned char *)params->stack,
        .stack_size = params->stack_size,
    };

    mutex_unlock(&params->sync);

    printf("Starting %s-Thread\n", (char *)(uintptr_t)arg);
    f12r_execute(&flow, arg, sizeof(intptr_t), &result, fn_name);
    printf("%s-Thread ran to completion res: %ld\n", (char *)arg, (long int)result);
    return NULL;
}

uint32_t user_bpf_thread_new(f12r_flow_t *flow, uint64_t *regs)
{
    bpf_thread_info_t *params = (bpf_thread_info_t *)(uintptr_t)regs[1];

    _thread_starter_params_t thread_params = {
        .sync = MUTEX_INIT_LOCKED,
        .stack = params->bpf_stack,
        .stack_size = params->bpf_stack_size,
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

f12r_call_t femto_container_user_helper(uint32_t num)
{
    switch (num) {
    case BPF_FUNC_USER_TEST_HELPER:
        return &user_test_helper;
    case BPF_FUNC_USER_FP_DISPATCH:
        return &user_fp_dispatch;
    case BPF_FUNC_MUTEX_NEW:
        return &user_bpf_mutex_new;
    case BPF_FUNC_MUTEX_LOCK:
        return &user_bpf_mutex_lock;
    case BPF_FUNC_MUTEX_UNLOCK:
        return &user_bpf_mutex_unlock;
    case BPF_FUNC_WAIT_MS:
        return &user_bpf_wait_ms;
    case BPF_FUNC_THREAD_NEW:
        return &user_bpf_thread_new;
    default:
        return NULL;
    }
}
#endif
