/*
 * SPDX-FileCopyrightText: 2026 Leonard Herbst
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 *
 * @file
 * @brief   Target of LLVM's BPF trap calls
 *
 * The LLVM BPF backend (since LLVM 21) lowers 'unreachable' IR instructions
 * to calls to __bpf_trap(). Undefined, it is an external symbol, which
 * gen_embpf rejects. Reaching it means the VM executes code the compiler
 * proved unreachable (undefined behaviour), so abort the VM.
 *
 * @author  Leonard Herbst <leonard.herbst@tu-dresden.de>
 */

#include "builtin_helpers.h"

__attribute__((used, noinline))
void __bpf_trap(void)
{
    bpf_builtin_abort_with_msg("virtwip: reached unreachable code (__bpf_trap)\n");
}

/** @} */
