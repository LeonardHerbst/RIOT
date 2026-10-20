#include "builtin_helpers.h"
#include "helpers.h"
#include "shared.h"
#include <stdint.h>

int main_func(call_ctx_t * ctx) {
    bpf_builtin_printf("Context struct size on eBPF VM: %ld\n", (long int) sizeof(*ctx));
    uint32_t test = bpf_user_fp_dispatch(ctx);
    return test;
}
