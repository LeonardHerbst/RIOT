#include "builtin_helpers.h"
#include "helpers.h"
#include "shared.h"

#define BPF_THREAD_STACKSIZE 512
#define OS_THREAD_STACKSIZE 4096

/* This mutex secures the CS */
static sys_mutex_t mutex;
/* This mutex is locked by the producer and unlocked by the consumer
 * RIOT allows unlocking from another thread.
 * This essentially used a Semaphore with a limit of 1. */
static sys_mutex_t queue;

__attribute__((aligned(8)))
static char bpf_stack_a[BPF_THREAD_STACKSIZE];
__attribute__((aligned(8)))
static char os_stack_a[OS_THREAD_STACKSIZE];
__attribute__((aligned(8)))
static char bpf_stack_b[BPF_THREAD_STACKSIZE];
__attribute__((aligned(8)))
static char os_stack_b[OS_THREAD_STACKSIZE];

static bpf_thread_info_t thread_a = {
        .bpf_stack = bpf_stack_a,
        .bpf_stack_size = BPF_THREAD_STACKSIZE,
        .os_stack = os_stack_a,
        .os_stack_size = OS_THREAD_STACKSIZE,
        .prio = 8,
        .vm_func = "thread_func_a",
        .arg = "Consume",
        .name = "VM Thread A",
    };

static bpf_thread_info_t thread_b = {
        .bpf_stack = bpf_stack_b,
        .bpf_stack_size = BPF_THREAD_STACKSIZE,
        .os_stack = os_stack_b,
        .os_stack_size = OS_THREAD_STACKSIZE,
        .prio = 8,
        .vm_func = "thread_func_b",
        .arg = "Produce",
        .name = "VM Thread B",
    };

int main_func(void) {
    bpf_mutex_new(&mutex, sizeof(sys_mutex_t));
    bpf_mutex_new(&queue, sizeof(sys_mutex_t));

    bpf_thread_new(&thread_a);
    bpf_wait_ms(250);
    bpf_thread_new(&thread_b);

    return 0;
}

__attribute__((used))
void thread_func_a(void *arg) {
    for (int i = 0; i < 16; i++) {
        bpf_mutex_lock(&mutex);
        bpf_builtin_printf("MSG: %s\n", (char *) arg);
        bpf_wait_ms(250);
        bpf_mutex_unlock(&mutex);
        bpf_mutex_lock(&queue);
    }
}

__attribute__((used))
void thread_func_b(void *arg) {
    for (int i = 0; i < 16; i++) {
        bpf_mutex_lock(&mutex);
        bpf_builtin_printf("MSG: %s\n", (char *) arg);
        bpf_wait_ms(250);
        bpf_mutex_unlock(&mutex);
        bpf_mutex_unlock(&queue);
    }
}
