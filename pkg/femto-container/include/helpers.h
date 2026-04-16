#ifndef FEMTO_HELPERS_H
#define FEMTO_HELPERS_H

#include "shared.h"
#include <stdint.h>
#include "femtocontainer/builtin_shared.h"

static void *(*bpf_printf)(const char *fmt, ...) = (void *) BPF_FUNC_BPF_PRINTF;
static uint32_t (*bpf_rand)(void) = (void *) BPF_FUNC_BPF_RAND;

#endif
