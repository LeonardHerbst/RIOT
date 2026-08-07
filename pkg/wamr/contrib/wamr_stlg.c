/*
 * SPDX-FileCopyrightText: 2026 TU Bergakademie Freiberg Karl Fessel
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     pkg_wamr_contrib
 * @{
 * @file
 *
 * @brief       Stage definitions and helpers for profiling the WAMR
 *              load/instantiate/execute pipeline.
 *
 * @author      Karl Fessel <karl.fessel@tu-freiberg.de>
 */
#include "wamr_stlg.h"

#ifdef LOG_INTERNALS
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "ztimer.h"

static const char *stages_wamr[] = {
    "load_start",
    "load_end",
    "parse_sections_start",
    "parse_sections_end",
    "prepare_functions_start",
    "prepare_functions_end",
    "instantiate_start",
    "instantiate_end",
};

static ztimer_now_t timestamps[sizeof(stages_wamr) / sizeof(char *)];
static stlg_state_t state;

void wamr_stlg_init(void)
{
    stlg_init(&state, ZTIMER_USEC, stages_wamr, timestamps,
              sizeof(stages_wamr) / sizeof(stages_wamr[0]));
}

void wamr_stlg_record(const char *stage)
{
    stlg_record_stage(&state, (char *)stage);
}

void wamr_stlg_print(void)
{
    stlg_as_csv(&state);
}
#endif
/** @} */
