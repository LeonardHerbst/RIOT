/*
 * SPDX-FileCopyrightText: 2015 Martine Lenders <mlenders@inf.fu-berlin.de>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @defgroup    pkg_lwip_opts   lwIP options
 * @ingroup     pkg_lwip
 * @brief       Options for the lwIP stack
 * @{
 *
 * @file
 * @brief   Option definitions
 *
 * @author  Martine Lenders <mlenders@inf.fu-berlin.de>
 */

#include <stdbool.h>
#include "lwipopts.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LWIP_PROVIDE_ERRNO 1

#ifdef DEVELHELP
void sys_mark_tcpip_thread(void);
#define LWIP_MARK_TCPIP_THREAD sys_mark_tcpip_thread

bool sys_check_core_locked(void);
#define LWIP_ASSERT_CORE_LOCKED() \
    LWIP_ASSERT("Core lock held", sys_check_core_locked())
#endif

#define LWIP_NO_INTTYPES_H 1
#define LWIP_NO_CTYPE_H 1

/** @} */

#ifdef __cplusplus
}
#endif

/** @} */
