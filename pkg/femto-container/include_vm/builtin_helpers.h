#ifndef FEMTO_HELPERS_H
#define FEMTO_HELPERS_H

#include <stdint.h>
#include "builtin_shared.h"

static void (*bpf_builtin_abort)(void) = (void *) BPF_FUNC_BUILTIN_ABORT;
static void (*bpf_builtin_abort_with_msg)(const char *msg) = (void *) BPF_FUNC_BUILTIN_ABORT_WITH_MSG;
static void (*bpf_builtin_printf)(const char *fmt, ...) = (void *) BPF_FUNC_BUILTIN_PRINTF;
static void (*bpf_builtin_sprintf)(char *s, const char *fmt, ...) = (void *) BPF_FUNC_BUILTIN_SPRINTF;
static int32_t (*bpf_builtin_atoi)(const char *s) = (void *) BPF_FUNC_BUILTIN_ATIO;
#endif
