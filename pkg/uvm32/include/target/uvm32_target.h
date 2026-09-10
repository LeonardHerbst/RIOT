#pragma once

#include <stddef.h>
#include <stdint.h>
#include "uvm32_sys.h"

static uint32_t syscall(uint32_t id, uint32_t param1, uint32_t param2) {
    register uint32_t a0 asm("a0") = (uint32_t)(param1);
    register uint32_t a1 asm("a1") = (uint32_t)(param2);
    register uint32_t a2 asm("a2");
    register uint32_t a7 asm("a7") = (uint32_t)(id);

    asm volatile (
        "ecall"
        : "=r"(a2) // output
        : "r"(a7), "r"(a0), "r"(a1) // input
        : "memory"
    );
    return a2;
}

#define syscall_cast(id, p1, p2) syscall((uint32_t)(id), (uint32_t)(p1), (uint32_t)(p2))

// The yield system call is always available
#define yield(x) syscall_cast(UVM32_SYSCALL_YIELD, x, 0)

#ifdef UVM32_DEFAULT_SYSCALLS

#define println(x)      syscall_cast(UVM32_SYSCALL_PRINTLN, x, 0)
#define print(x)        syscall_cast(UVM32_SYSCALL_PRINT, x, 0)
#define printdec(x)     syscall_cast(UVM32_SYSCALL_PRINTDEC, x, 0)
#define printhex(x)     syscall_cast(UVM32_SYSCALL_PRINTHEX, x, 0)
#define millis()        syscall_cast(UVM32_SYSCALL_MILLIS, 0, 0)
#define printbuf(x, y)  syscall_cast(UVM32_SYSCALL_PRINTBUF, x, y)
#define rand()          syscall_cast(UVM32_SYSCALL_RAND, 0, 0)

#endif // UVM32_DEFAULT_SYSCALLS
