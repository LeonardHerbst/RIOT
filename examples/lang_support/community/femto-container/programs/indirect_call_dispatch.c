#include <stddef.h>
#include <stdint.h>

#ifndef REGULAR
#  include "builtin_helpers.h"
#  define log(...) bpf_builtin_printf(__VA_ARGS__)
#else
#  include <stdio.h>
#  define log(...) printf(__VA_ARGS__)
#endif /* ifndef REGULAR */

void do_work(void);

#ifndef REGULAR
int main_func(void) {
#else
int main(void) {
#endif
do_work();
return 1;
}

typedef enum {
    ICD_FUNCTION_ADD,
    ICD_FUNCTION_SUB,
    ICD_FUNCTION_CAPS,
    ICD_FUNCTION_DECAPS
} function_id_t;

typedef function_id_t alg_fn;
typedef function_id_t str_fn;

int32_t add(int32_t a, int32_t b) {
    return a + b;
}

int32_t sub(int32_t a, int32_t b) {
    return a - b;
}

void caps(char *s)
{
    for (; *s; s++) {
        if (*s >= 'a' && *s <= 'z') {
            *s -= 32;
        }
    }
}

void decaps(char *s)
{
    for (; *s; s++) {
        if (*s >= 'A' && *s <= 'Z') {
            *s += 32;
        }
    }
}

size_t indirect_call_dispatch(function_id_t id, size_t arg1, size_t arg2) {
    switch (id) {
        case ICD_FUNCTION_ADD:
            return (size_t) add((int32_t) arg1, (int32_t) arg2);
        case ICD_FUNCTION_SUB:
            return (size_t) add((int32_t) arg1, (int32_t) arg2);
        case ICD_FUNCTION_CAPS:
            caps((char*) arg1);
            return 0;
        case ICD_FUNCTION_DECAPS:
            decaps((char*) arg1);
            return 0;
        default:
            log("Unknown function with id %ld", (long int) id);
    }
}

struct mock_structure {
    alg_fn afn;
    str_fn sfn;
};

void do_work(void) {
    struct mock_structure struct_1 = {
        .afn = ICD_FUNCTION_ADD,
        .sfn = ICD_FUNCTION_CAPS,
    };
    int32_t x = 42;
    log("x: %ld\n", (long int) x);
    x = (int32_t) indirect_call_dispatch((function_id_t) struct_1.afn, 1300, 37);
    log("x: %ld\n", (long int) x);
    char y[] = "Hello, World!";
    log("y: %s\n", y);
    (void) indirect_call_dispatch((function_id_t) struct_1.sfn, (size_t) y, 0);
    log("y: %s\n", y);
}
