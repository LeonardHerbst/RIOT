#pragma once

// Common definitions used by both uvm32.c and any code running inside VM

// System provided UVM32_SYSCALLs, start at 0x1000000
#define UVM32_SYSCALL_HALT          0x1000000
#define UVM32_SYSCALL_YIELD         0x1000001
#define UVM32_SYSCALL_STACKPROTECT  0x1000002

// Address of External RAM, when offered by host
#define UVM32_EXTRAM_BASE 0x10000000

#define UVM32_BUILTIN_SYSCALL_START 0x1000000

#ifdef UVM32_DEFAULT_SYSCALLS
#  define UVM32_SYSCALL_PRINT       0x2000000
#  define UVM32_SYSCALL_PRINTLN     0x2000001
#  define UVM32_SYSCALL_PRINTDEC    0x2000002
#  define UVM32_SYSCALL_PRINTHEX    0x2000003
#  define UVM32_SYSCALL_MILLIS      0x2000004
#  define UVM32_SYSCALL_PRINTBUF    0x2000005
#  define UVM32_SYSCALL_RAND        0x2000006
#endif // UVM32_DEFAULT_SYSCALLS
