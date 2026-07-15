#include "lua_stlg.h"

static const char*  stages_lua[] = {
    "lua_load_start",
    "lua_load_end",
    "luaL_loadbufferx_start",
    "luaL_loadbufferx_end",
    "f_parser_start",
    "f_parser_end",
    "luaY_parser_start",
    "luaY_parser_end",
    "mainfunc_start",
    "mainfunc_end",
};

static ztimer_now_t timestamps[sizeof(stages_lua)/sizeof(char *)];

static stlg_state_t state;

void lua_print_stages(void) {
    stlg_print_stages(&state);
}

void lua_record_stage(char *stage) {
    stlg_record_stage(&state, stage);
}

void lua_init_stlg(void) {
    stlg_init(&state, ZTIMER_USEC, stages_lua, timestamps, sizeof(stages_lua)/sizeof(char *));
}
