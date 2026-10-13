#include "builtin_helpers.h"
#include "helpers.h"
#include <stdint.h>

static const char *msg = "Hello, World!";
static int num = 42;

int main_func(void) {
    uint32_t test = bpf_user_test_helper();
    return test;
}
