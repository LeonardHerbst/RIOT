#ifdef LOG_INTERNALS
#include "lua_stlg.h"

static const char*  stages_lua[] = {
    "load_start",
    "load_end",
    "parsecompile_start",
    "parsecompile_end"
};

static ztimer_now_t timestamps[sizeof(stages_lua)/sizeof(char *)];

static stlg_state_t state;

void lua_stlg_print(void) {
    stlg_as_csv(&state);
}

void lua_stlg_record(const char *stage) {
    stlg_record_stage(&state, (char *) stage);
}

void lua_stlg_init(void) {
    stlg_init(&state, ZTIMER_USEC, stages_lua, timestamps, sizeof(stages_lua)/sizeof(char *));
}
#endif
