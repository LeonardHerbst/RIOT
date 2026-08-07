/*
 * SPDX-FileCopyrightText: 2026 TU Bergakademie Freiberg Karl Fessel
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @defgroup     pkg_wamr_contrib WAMR stage logger glue
 * @ingroup      pkg_wamr
 * @brief        Glue code that exposes RIOT's stage_logger to WAMR's
 *               internal load/instantiate/execute pipeline.
 *
 * Each LOG_STAGE(<name>) call records a timestamp for the stage name
 * "<name>" using RIOT's @ref sys_stage_logger module.  The
 * corresponding wamr_stlg_init() / wamr_stlg_print() functions are
 * defined in wamr_stlg.c and are expected to be invoked from
 * application code (or via @ref pkg_wamr "pkg/wamr" helper code) to
 * initialize the stage list and to dump the collected timestamps.
 *
 * When LOG_INTERNALS is not defined, the macros compile to no-ops so
 * the WAMR library can be used unmodified.
 */

#ifndef WAMR_STLG_H
#define WAMR_STLG_H

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

void wamr_stlg_init(void);
void wamr_stlg_record(const char *stage);
void wamr_stlg_print(void);

#  define LOG_STAGE_INIT()      wamr_stlg_init()
#  define LOG_STAGE(stage)      wamr_stlg_record(#stage)
#  define LOG_STAGE_PRINT()     wamr_stlg_print()
#endif

#ifdef __cplusplus
}
#endif

#endif /* WAMR_STLG_H */
