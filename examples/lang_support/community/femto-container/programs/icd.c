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

#define ICD_CALL_ONE_ARG(name, a)            icd_dispatch((function_id_t) name, (uint64_t) a, 0, 0, 0)
#define ICD_CALL_TWO_ARGS(name, a, b)        icd_dispatch((function_id_t) name, (uint64_t) a, (uint64_t) b, 0, 0)
#define ICD_CALL_THREE_ARGS(name, a, b, c)   icd_dispatch((function_id_t) name, (uint64_t) a, (uint64_t) b, (uint64_t) c, 0)
#define ICD_CALL_FOUR_ARGS(name, a, b, c, d) icd_dispatch((function_id_t) name, (uint64_t) a, (uint64_t) b, (uint64_t) c, (uint64_t) d)

#define ICD_GET_VARIANT(_0, _1, _2, _3, _4, SELECT, ...) SELECT

/* The __VA_ARGS__ shift the options in the argument list. So with:
 * name + 1 arg: _2 => ICD_CALL_FOUR_ARGS, _3 => ICD_CALL_THREE_ARGS, _4 => ICD_CALL_TWO_ARGS, SELECT => ICD_CALL_ONE_ARG
 * name + 3 arg: _4 => ICD_CALL_FOUR_ARGS, SELECT => ICD_CALL_THREE_ARGS */
#define icd_call(...) ICD_GET_VARIANT(__VA_ARGS__, ICD_CALL_FOUR_ARGS, ICD_CALL_THREE_ARGS, ICD_CALL_TWO_ARGS, ICD_CALL_ONE_ARG)(__VA_ARGS__)

#define icd_get(name) ICD_FUNCTION_##name

#define icd_register(name) ICD_FUNCTION_##name

typedef enum {
    icd_register(add),
    icd_register(sub),
    icd_register(caps),
    icd_register(decaps)
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

uint64_t icd_dispatch(function_id_t id, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
    (void) arg3;
    (void) arg4;
    switch (id) {
        case icd_get(add):
            return (size_t) add((int32_t) arg1, (int32_t) arg2);
        case icd_get(sub):
            return (size_t) sub((int32_t) arg1, (int32_t) arg2);
        case icd_get(caps):
            caps((char*) arg1);
            return 0;
        case icd_get(decaps):
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
        .afn = icd_get(add),
        .sfn = icd_get(caps),
    };
    int32_t x = 0;
    log("x: %ld\n", (long int) x);
    x = (int32_t) icd_call(struct_1.afn, 1300, 37);
    struct_1.afn = icd_get(sub);
    log("x: %ld\n", (long int) x);
    x = (int32_t) icd_call(struct_1.afn, x, 1295);
    log("x: %ld\n", (long int) x);
    char y[] = "Hello, World!";
    log("y: %s\n", y);
    (void) icd_call(struct_1.sfn, y);
    log("y: %s\n", y);
}
