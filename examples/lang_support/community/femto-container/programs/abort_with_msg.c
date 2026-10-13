#include "builtin_helpers.h"

int main_func(void) {
    bpf_builtin_abort_with_msg("Error message!\n");
    return 2;
}
