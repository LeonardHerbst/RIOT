#include <stdio.h>

#include "stage_logger.h"
#include "ztimer.h"

static const char *stages[4] = {
    "Stage1",
    "Stage2",
    "Stage3",
    "Stage4"
};

static ztimer_now_t tss[sizeof(stages)/sizeof(char *)];

static stlg_state_t state;

int main(void)
{
    stlg_init(&state, ZTIMER_USEC, stages, tss, sizeof(stages)/sizeof(char *));
    stlg_record_stage(&state, "Stage1");
    puts("Recorded Stage1");
    ztimer_sleep(ZTIMER_USEC, 2000000);
    stlg_record_stage(&state, "Stage2");
    puts("Recorded Stage2");
    ztimer_sleep(ZTIMER_USEC, 2000000);
    stlg_record_stage(&state, "Stage3");
    puts("Recorded Stage3");
    stlg_print_stages(&state);
    return 0;
}
