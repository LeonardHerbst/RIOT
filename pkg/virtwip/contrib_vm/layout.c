/*
 * SPDX-FileCopyrightText: 2026 Leonard Herbst
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 *
 * @file
 * @brief   VM side of the host/VM layout check
 *
 * @author  Leonard Herbst <leonard.herbst@tu-dresden.de>
 */

#include "lwip/opt.h"

#if VIRTWIP_LAYOUT_CHECK

#include "virtwip_layout.h"

/* Writes the VM's value of every entry in virtwip_layout_list.h into
 * a->values (list order) and returns the number of entries. Entries beyond
 * VIRTWIP_LAYOUT_MAX are counted but not written; the host reports that. */
uint32_t virtwip_layout_info(virtwip_layout_args_t *a)
{
    uint32_t n = 0;

#define VIRTWIP_LAYOUT_SIZE(type) \
    if (n < VIRTWIP_LAYOUT_MAX) { a->values[n] = (uint32_t)sizeof(type); } n++;
#define VIRTWIP_LAYOUT_OFFSET(type, member) \
    if (n < VIRTWIP_LAYOUT_MAX) { a->values[n] = (uint32_t)offsetof(type, member); } n++;
#include "virtwip_layout_list.h"
#undef VIRTWIP_LAYOUT_SIZE
#undef VIRTWIP_LAYOUT_OFFSET

    return n;
}

#endif /* VIRTWIP_LAYOUT_CHECK */

/** @} */
