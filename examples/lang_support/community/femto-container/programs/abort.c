#include "builtin_helpers.h"

int main_func(void) {
    bpf_builtin_abort();
    return 2;
}
