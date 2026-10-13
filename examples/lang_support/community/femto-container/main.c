/*
 * Copyright (c) 2021 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       BPF tutorial 1 example
 *
 * @author      Koen Zandberg <koen.zandberg@inria.fr>
 *
 * @}
 */
#include <stdio.h>
#include <stdint.h>
#include "ztimer.h"

#include "femtocontainer/femtocontainer.h"
#include "programs/shared.h"

#include "blob/program.bin.h"

/* Pre-allocated stack for the virtual machine */
static uint8_t _stack[FEMTO_CONTAINER_STACK_SIZE] = { 0 };

static void fc_print_status(int code)
{
    switch (code) {
        case FC_OK:
            puts("Femto-Container: OK");
            break;
        case FC_ILLEGAL_INSTRUCTION:
            puts("Femto-Container error: illegal instruction");
            break;
        case FC_ILLEGAL_MEM:
            puts("Femto-Container error: illegal memory access");
            break;
        case FC_ILLEGAL_JUMP:
            puts("Femto-Container error: illegal jump");
            break;
        case FC_ILLEGAL_CALL:
            puts("Femto-Container error: illegal call");
            break;
        case FC_ILLEGAL_LEN:
            puts("Femto-Container error: illegal length");
            break;
        case FC_ILLEGAL_REGISTER:
            puts("Femto-Container error: illegal register");
            break;
        case FC_NO_RETURN:
            puts("Femto-Container error: no return from program");
            break;
        case FC_OUT_OF_BRANCHES:
            puts("Femto-Container error: out of branches");
            break;
        case FC_ILLEGAL_DIV:
            puts("Femto-Container error: illegal division");
            break;
        case FC_CALL_DEPTH_LIMIT:
            puts("Femto-Container error: reached max call depth");
            break;
        case FC_OUT_OF_STACK:
            puts("Femto-Container error: out of stack");
            break;
        case FC_MISSING_PREFLIGHT:
            puts("Femto-Container error: no preflight validation performed");
            break;
        case FC_ILLEGAL_ENTRY_POINT:
            puts("Femto-Container error: the entry function passed to f12r_run does not exist in the eBPF program");
            break;
        case FC_ABORT:
            puts("Vm code called abort");
            break;
        default:
            printf("Femto-Container: unknown status code %d\n", code);
            break;
    }
}

uint32_t test_function(void) {
    printf("This is the test function!\n");
    return 1337;
}

call_ctx_t ctx = { .fp = &test_function };

int main(void)
{
    puts("=== Program Start ===");
    /* Define the application */
    f12r_t f12r_container = {
        .application = program_bin,             /* The increment.bin content */
        .application_len = sizeof(program_bin), /* Length of the application */
        .mut_regions = NULL,                      /* NULL means femtocontainer will allocate */
        .mut_regions_len = 0,                     /* 0 length */
        .stack = _stack,                          /* Preallocated stack */
        .stack_size = sizeof(_stack),             /* And the length */
        .branches_remaining = UINT32_MAX,
    };

    int64_t result = 0;

    puts("\tFemto Setup");
    f12r_setup(&f12r_container);

    // f12r_mem_region_t region;
    // f12r_add_region(&f12r_container, &region, &ctx, sizeof(context), FC_MEM_REGION_READ | FC_MEM_REGION_WRITE);

    int res;
    puts("\tFemto Preflight");
    if((res = f12r_verify_preflight(&f12r_container)) != FC_OK) {
        goto error;
    }

    puts("\tFemto Start Execution");
    if ((res = f12r_execute_ctx(&f12r_container, &ctx, sizeof(call_ctx_t), &result, "main_func")) != FC_OK) {
        goto error;
    }

    printf("\tFemto Completed Execution (returned %ld)\n", (unsigned long)result);

    puts("\tFemto Clean Up");
    f12r_clean_up(&f12r_container);

    puts("=== Program End ===");
    return 0;
error:
      f12r_clean_up(&f12r_container);
      fc_print_status(res);
}
