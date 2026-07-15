#ifndef LOG_INTERNALS
#  define LOG_STAGE(x)
#  define INIT_STAGE_LOGGER
#else
#include "ztimer.h"
#include "stage_logger.h"

void lua_init_stlg(void);
void lua_record_stage(char *stage);
void lua_print_stages(void);

#define INIT_STAGE_LOGGER lua_init_stlg();

#define LOG_STAGE(stg) lua_record_stage(#stg);

#define PRINT_STAGES lua_print_stages();

#endif
