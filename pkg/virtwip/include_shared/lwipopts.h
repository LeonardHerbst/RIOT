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
 * @brief   Option definitions, shared by the host glue and the VM image
 *
 * The lwIP configuration itself comes from the generated virtwip_config.h
 * (see pkg/virtwip/Makefile.conf), so both sides compile lwIP's headers with
 * identical options and therefore identical struct layouts.
 *
 * VIRTWIP_VM is defined only when compiling code for the VM image
 * (pkg/virtwip/Makefile). Use it for options that must differ between the
 * two sides and do not affect any data layout shared across the border,
 * e.g. replacements for libc facilities the VM does not have.
 *
 * @author  Martine Lenders <mlenders@inf.fu-berlin.de>
 */

#include <stdbool.h>

#include "virtwip_config.h"
/* bpfptr_t and __bpf_shared_ptr(), used by the patched lwIP headers */
#include "builtin_shared.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef VIRTWIP_VM
/* The VM has no libc: lwIP provides errno values and avoids
 * inttypes.h/ctype.h. The host uses its libc instead. */
#define LWIP_PROVIDE_ERRNO 1
#define LWIP_NO_INTTYPES_H 1
#define LWIP_NO_CTYPE_H 1

#ifdef DEVELHELP
void sys_mark_tcpip_thread(void);
#define LWIP_MARK_TCPIP_THREAD sys_mark_tcpip_thread

bool sys_check_core_locked(void);
#define LWIP_ASSERT_CORE_LOCKED() \
    LWIP_ASSERT("Core lock held", sys_check_core_locked())
#endif /* DEVELHELP */
#else /* VIRTWIP_VM */
/* The host glue uses the errno values of its libc */
#define LWIP_ERRNO_STDINCLUDE 1
#endif /* VIRTWIP_VM */

/** @} */

#ifdef __cplusplus
}
#endif

/** @} */
