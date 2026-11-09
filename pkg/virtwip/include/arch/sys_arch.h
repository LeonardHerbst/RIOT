/*
 * SPDX-FileCopyrightText: 2015 Martine Lenders <mlenders@inf.fu-berlin.de>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @defgroup    pkg_lwip_sys    Porting layer
 * @ingroup     pkg_lwip
 * @brief       System abstraction layer
 * @{
 *
 * @file
 * @brief   OS abstraction layer
 * @see     https://www.nongnu.org/lwip/2_1_x/group__sys__os.html
 *
 * @author  Martine Lenders <mlenders@inf.fu-berlin.de>
 */

#include <stdbool.h>
#include <stdint.h>

#include "shared_defs.h"

/* lwIP's sys.h requires a SYS_ARCH_PROTECT implementation. lwIP itself runs
 * in the VM (include_vm/arch/sys_arch.h); the host glue never enters a
 * protected section, these definitions only mirror the VM's to keep lwIP's
 * headers consistent. */
#define SYS_ARCH_PROTECT(x)         sys_mutex_lock(&(x))
#define SYS_ARCH_UNPROTECT(x)       sys_mutex_unlock(&(x))
#define SYS_ARCH_DECL_PROTECT(x)    sys_mutex_t x; sys_mutex_new(&(x))

/** @} */
