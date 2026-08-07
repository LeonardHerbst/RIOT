#include "stage_logger.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int stlg_init(stlg_state_t *stlgs, ztimer_clock_t *clock, const char **stages, ztimer_now_t *timestamps, size_t n_stages)
{
    stlgs->clock = clock;
    stlgs->stages = stages;
    stlgs->timestamps = timestamps;
    stlgs->n_stages = n_stages;

    memset(stlgs->timestamps, 0, stlgs->n_stages * sizeof(ztimer_now));
    return 0;
}

static int _get_stage_index(stlg_state_t *stlgs, char *stage) {
    int stage_index = -1;
    for (size_t i = 0; i < stlgs->n_stages; i++) {
        if (strcmp(stlgs->stages[i], stage) == 0) {
            stage_index = (int)i;
            break;
        }
    }
    return stage_index;
}

int stlg_record_stage(stlg_state_t *stlgs, char *stage)
{
    int stage_index = _get_stage_index(stlgs, stage);
    if (stage_index == -1) {
        printf("Error: Stage %s is unknown!\n", stage);
        return -1;
    }
    stlgs->timestamps[stage_index] = ztimer_now(stlgs->clock);
    return 0;
}

ztimer_now_t stlg_get_ts(stlg_state_t *stlgs, char *stage)
{
    int stage_index = _get_stage_index(stlgs, stage);
    if (stage_index == -1) {
        return 0;
    }
    return stlgs->timestamps[stage_index];
}

void stlg_as_json(stlg_state_t *stlgs)
{
    printf("{");
    for (size_t i = 0; i < stlgs->n_stages; i++) {
        printf("\"%s\": %lu", stlgs->stages[i], (unsigned long int) stlgs->timestamps[i]);
        printf((i + 1 < stlgs->n_stages) ? ", ": "");
    }
    printf("}\n");
}

void stlg_as_csv(stlg_state_t *stlgs)
{
    for (size_t i = 0; i < stlgs->n_stages; i++) {
        printf("%s%s", stlgs->stages[i], i + 1 < stlgs->n_stages ? ";": "\n");
    }
    for (size_t i = 0; i < stlgs->n_stages; i++) {
        printf("%lu%s", (unsigned long int) stlgs->timestamps[i], i + 1 < stlgs->n_stages ? ";": "\n");
    }
}
