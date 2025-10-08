/*
 * SPDX-FileCopyrightText: 2025 TU Dresden
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @ingroup     drivers_paa5100je
 *
 * @{
 * @file
 * @brief       Internal addresses, registers and constants
 *
 * @author      Leonard Herbst <leonard.herbst@tu-dresden.de>
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Register definitions for the PAA5100JE amd PMW3901 optical flow sensors
 * @{
 */
/** Product ID (reset value: 0x49, read-only) */
#define REG_ID                 (0x00)

/** Product revision ID (reset value: 0x00, read-only) */
#define REG_REV                (0x01)

/** Motion data ready flag (bitfield, indicates new data availability, read/write) */
#define REG_DATA_READY         (0x02)

/** Motion burst register: block read of motion, delta X/Y, quality, shutter, etc., read-only */
#define REG_MOTION_BURST       (0x16)

/** Write 0x5A to reset internal registers/state machine (soft reset), write-only */
#define REG_POWER_UP_RESET     (0x3A)

/** Sensor orientation/mirroring (read/write, flips axes depending on mounting) */
#define REG_ORIENTATION        (0x5B)

/** Inverse Product ID (reset value: 0xB6, bitwise NOT of REG_ID, read-only) */
#define REG_INV_ID             (0x5F)
/** @} */

#ifdef __cplusplus
}
#endif

/** @} */
