#include "builtin_helpers.h"

static const char *msg = "Hello, World!";
static int num = 42;

int main_func(void) {
    bpf_builtin_printf("Num: %d msg: %s\n", num, msg);
    return 1;
}
