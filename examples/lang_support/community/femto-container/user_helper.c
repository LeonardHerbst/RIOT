#ifdef FEMTO_CONTAINER_USER_HELPER
#include <stdio.h>

#include "femtocontainer/femtocontainer.h"
#include "programs/shared.h"

uint32_t user_test_helper(f12r_t *f12r, uint64_t *regs)
{
    (void)f12r;
    (void)regs;
    printf("User test helper called\n");
    return 42;
}

uint32_t user_fp_dispatch(f12r_t *f12r, uint64_t *regs) {
    (void)f12r;
    call_ctx_t *ctx = (call_ctx_t *)(long)regs[1];
    printf("Context struct size on the host: %lld\n", (long long int) sizeof(*ctx));
    return ctx->fp();
}

f12r_call_t femto_container_user_helper(uint32_t num)
{
    switch (num) {
        case BPF_FUNC_USER_TEST_HELPER:
            return &user_test_helper;
        case BPF_FUNC_USER_FP_DISPATCH:
            return &user_fp_dispatch;
        default:
            return NULL;
    }
}
#endif
