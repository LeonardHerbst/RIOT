#pragma once

#include "ztimer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ztimer_clock_t *clock;
    const char **stages;
    ztimer_now_t *timestamps;
    size_t n_stages;
} stlg_state_t;

int stlg_init(stlg_state_t *stlgs, ztimer_clock_t *clock, const char **stages, ztimer_now_t *timestamps, size_t n_stages);

int stlg_record_stage(stlg_state_t *stlgs, char *stage);

ztimer_now_t stlg_get_ts(stlg_state_t *stlgs, char *stage);

void stlg_as_json(stlg_state_t *stlgs);

void stlg_as_csv(stlg_state_t *stlgs);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */
