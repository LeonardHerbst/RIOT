#pragma once

#include "uvm32_sys.h"
#include "uvm32.h"

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include "random.h"
#include "ztimer.h"

#ifndef UVM32_STEPSIZE
#  define UVM32_STEPSIZE UINT32_MAX
#endif // !UVM32_STEPSIZE

#define UVM32_FLAG_INIT_DONE 0x01
#define UVM32_FLAG_LOAD_DONE 0x02

enum {
    UVM32_OK = 0,
    UVM32_MISSING_INIT = -1,
    UVM32_MISSING_LOAD = -2,
    UVM32_RUNTIME_ERROR = -3,
    UVM32_UNKNOWN_SYSCALL = -4,
    UVM32_INSUFFICIENT_MEMORY = -5,
};

typedef bool (*syscall_handler)(uint32_t, uvm32_state_t*, uvm32_evt_t*, uint32_t*);

typedef struct {
    uvm32_state_t state;
    syscall_handler syscall_handler;
    uint8_t flags;
    const uint8_t *program;
    const uint32_t program_len;
} uvm32_instance_t;

static uint32_t uvm32_instance_init(uvm32_instance_t* instance)
{
    instance->flags = 0;
    uvm32_init(&instance->state);
    instance->flags |= UVM32_FLAG_INIT_DONE;
    if (!uvm32_load(&instance->state, instance->program, (int) instance->program_len)){
        return UVM32_INSUFFICIENT_MEMORY;
    }
    instance->flags |= UVM32_FLAG_LOAD_DONE;
    return UVM32_OK;
}

static uint32_t uvm32_instance_execute(uvm32_instance_t *instance)
{
    if ((instance->flags & UVM32_FLAG_INIT_DONE) == 0) {
        return UVM32_MISSING_INIT;
    }
    if ((instance->flags & UVM32_FLAG_LOAD_DONE) == 0) {
        return UVM32_MISSING_LOAD;
    }

    uint32_t res = UVM32_OK;
    bool running = true;
    uvm32_evt_t last_event;
    ztimer_now_t start = ztimer_now(ZTIMER_MSEC);

    while(running) {
        uvm32_run(&instance->state, &last_event, UVM32_STEPSIZE);

        if (last_event.typ == UVM32_EVT_END) {
            running = false;
            break;
        }
        if (last_event.typ == UVM32_EVT_ERR) {
            printf("UVM32_EVT_ERR '%s' (%d)\n", last_event.data.err.errstr, (int)last_event.data.err.errcode);
            res = UVM32_RUNTIME_ERROR;
            break;
        }

        uint32_t syscall_type = last_event.data.syscall.code;
        if (syscall_type < UVM32_BUILTIN_SYSCALL_START) {
            if (instance->syscall_handler == NULL) {
                    printf("Unknown external syscall with id 0x%08x\n", syscall_type);
                    res = UVM32_UNKNOWN_SYSCALL;
                    running = false;
                    break;
            }
            running = instance->syscall_handler(syscall_type, &instance->state, &last_event, &res);
        } else {
            /* Syscalls with an ID higher than this are built in syscalls */
            switch (syscall_type) {
                case UVM32_SYSCALL_YIELD:
                    break;
#ifdef UVM32_DEFAULT_SYSCALLS
                case UVM32_SYSCALL_PRINT:
                    printf("%s", uvm32_arg_getcstr(&instance->state, &last_event, ARG0));
                    break;
                case UVM32_SYSCALL_PRINTLN:
                    printf("%s\n", uvm32_arg_getcstr(&instance->state, &last_event, ARG0));
                    break;
                case UVM32_SYSCALL_PRINTDEC:
                    printf("%lu\n", (unsigned long int)uvm32_arg_getval(&instance->state, &last_event, ARG0));
                    break;
                case UVM32_SYSCALL_PRINTHEX:
                    printf("%lx\n", (unsigned long int)uvm32_arg_getval(&instance->state, &last_event, ARG0));
                    break;
                case UVM32_SYSCALL_MILLIS:
                    uvm32_arg_setval(&instance->state, &last_event, RET, start - ztimer_now(ZTIMER_MSEC));
                    break;
                case UVM32_SYSCALL_PRINTBUF:
                    {
                        uvm32_slice_t buf = uvm32_arg_getslice(&instance->state, &last_event, ARG0, ARG1);
                        while(buf.len--) {
                            printf("%02x", *buf.ptr++);
                        }
                    }
                    break;
                case UVM32_SYSCALL_RAND:
                    uvm32_arg_setval(&instance->state, &last_event, RET, random_uint32());
                    break;
#endif // UVM32_DEFAULT_SYSCALLS
                default:
                    printf("Unknown builtin syscall with id 0x%08x\n", syscall_type);
                    res = UVM32_UNKNOWN_SYSCALL;
                    running = false;
                    break;
            }
        }
    }
    return res;
}
