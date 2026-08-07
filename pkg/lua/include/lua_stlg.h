#ifndef LUA_STLG_H
#define LUA_STLG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LOG_INTERNALS
#  define LOG_STAGE(stage)      do { } while (0)
#  define LOG_STAGE_INIT()      do { } while (0)
#  define LOG_STAGE_PRINT()     do { } while (0)
#else
#include "ztimer.h"
#include "stage_logger.h"

void lua_stlg_init(void);
void lua_stlg_record(const char *stage);
void lua_stlg_print(void);

#  define LOG_STAGE_INIT()      lua_stlg_init()
#  define LOG_STAGE(stage)      lua_stlg_record(#stage)
#  define LOG_STAGE_PRINT()     lua_stlg_print()
#endif

#ifdef __cplusplus
}
#endif

#endif /* LUA_STLG_H */
