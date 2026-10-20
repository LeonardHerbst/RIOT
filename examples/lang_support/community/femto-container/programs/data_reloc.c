#include "builtin_helpers.h"

static char bss[5];
static char data[] = "!@#$%";
const static char rodata[] = ".rodata";

struct wrapper {
    char *bss;
    char *data;
    const char* rodata;
};

struct wrapper instance = {
    .bss = bss,
    .data = data,
    .rodata = rodata,
};

int main_func(void) {
    instance.bss[0] = '.';
    instance.bss[1] = 'b';
    instance.bss[2] = 's';
    instance.bss[3] = 's';
    instance.data[0] = '.';
    instance.data[1] = 'd';
    instance.data[2] = 'a';
    instance.data[3] = 't';
    instance.data[4] = 'a';
    bpf_builtin_printf("bss: %s\ndata: %s\nrodata: %s\n",
            instance.bss,
            instance.data,
            instance.rodata
            );
    return 0;
}
