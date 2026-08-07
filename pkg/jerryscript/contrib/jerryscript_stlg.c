#include "jerryscript_stlg.h"

#ifdef LOG_INTERNALS
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "ztimer.h"

static const char *stages_jerryscript[] = {
    "load_start",
    "load_end",
    "scan_all_start",
    "scan_all_end",
    "parse_function_args_start",
    "parse_function_args_end",
    "create_variables_start",
    "create_variables_end",
    "parse_statement_start",
    "parse_statement_end"
};

static ztimer_now_t timestamps[sizeof(stages_jerryscript) / sizeof(char *)];
static stlg_state_t state;

void jerryscript_stlg_init(void)
{
    stlg_init(&state, ZTIMER_USEC, stages_jerryscript, timestamps,
              sizeof(stages_jerryscript) / sizeof(stages_jerryscript[0]));
}

void jerryscript_stlg_record(const char *stage)
{
    stlg_record_stage(&state, (char *)stage);
}

void jerryscript_stlg_print(void)
{
    stlg_as_csv(&state);
}
#endif
/** @} */
