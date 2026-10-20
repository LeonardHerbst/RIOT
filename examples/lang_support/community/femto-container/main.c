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

uint32_t test_function(void) {
    printf("This is the test function!\n");
    return 1337;
}

call_ctx_t ctx = { .fp = &test_function };

/* Define the application */
static f12r_t f12r_container = {
    .application = program_bin,             /* The increment.bin content */
    .application_len = sizeof(program_bin), /* Length of the application */
    .mutable_data = NULL,                   /* NULL means femtocontainer will allocate */
    .mutable_data_len = 0,                  /* 0 length */
    .branches_remaining = UINT32_MAX,
};

static f12r_flow_t main_flow = {
    .f12r = &f12r_container,
    .stack = _stack,                        /* Preallocated stack */
    .stack_size = sizeof(_stack),           /* And the length */
};

static int64_t result = 0;

int main(void)
{
    puts("=== Program Start ===");

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
    if ((res = f12r_execute(&main_flow, &ctx, sizeof(call_ctx_t), &result, "main_func")) != FC_OK) {
        goto error;
    }

    printf("\tFemto Completed Execution (returned %ld)\n", (unsigned long)result);

    puts("\tFemto Clean Up");
    f12r_clean_up(&f12r_container);

    puts("=== Program End ===");
    return 0;
error:
    puts("\tFemto Clean Up");
    f12r_clean_up(&f12r_container);
    f12r_clean_up(&f12r_container);
}
