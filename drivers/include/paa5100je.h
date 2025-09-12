/*
 * SPDX-FileCopyrightText: 2025 TU Dresden
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @defgroup    drivers_paa5100je PAA5100JE
 * @ingroup     drivers_sensors
 * @brief       Optical Flow Sensor
 *
 * @{
 *
 * @file
 *
 * @author      Leonard Herbst <leonard.herbst@tu-dresden.de>
 */

#include <periph/gpio.h>
#include <periph/spi.h>

#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Variant of the sensor
 */
typedef enum {
    PAA5100JE = 0,
    PMW3901   = 1,
} paa5100je_variant_t;


/**
 * @brief   Device initialization parameters
 */
typedef struct {
    spi_t spi;                /**< SPI bus used */
    spi_clk_t clk;            /*< clock speed used on the selected SPI bus */
    gpio_t cs;                /**< pin connected to the chip select line */
    paa5100je_variant_t var;  /**< variant of the sensor used */
} paa5100je_params_t;

/**
 * @brief   Device descriptor for the driver
 */
typedef struct {
    const paa5100je_params_t *params;  /**< Device initialization parameters */
} paa5100je_t;

/**
 * @brief   Initialize the given device
 *
 * @param[inout]  dev      Device descriptor of the driver
 * @param[in]     params   Initialization parameters
 *
 * @retval        0        on success
 * @retval        -ENXIO   invalid SPI device
 * @retval        -EINVAL  invalid SPI CS pin/line or wrong device id or revision
 */
int paa5100je_init(paa5100je_t *dev, const paa5100je_params_t *params);

/**
 * @brief   Reads the relative motition vector from the device using burst read.
 *
 * Reads twelve bytes from the burst register.
 * Repeats the read when the data is not ready, the quality is too low
 * or the shutter values is too high.
 *
 * @param[in]     dev      device descriptor
 * @param[out]    x        x component
 * @param[out]    y        y component
 *
 * @retval        0        on success
 * @retval        -ETIME   data was not ready in timem, the quality was not good enough,
 *                         or the shutter to high
 */
int paa5100je_get_motion_burst(const paa5100je_t *dev, int16_t *x, int16_t *y);

/*
 * @brief   Reads the relative motition vector from the device.
 *
 * Reads five individually addressed registers from the device. Repeats this until data is ready.
 *
 * @param[in]     dev      device descriptor
 * @param[out]    x        x component
 * @param[out]    y        y component
 *
 * @retval        0        on success
 * @retval        -ETIME   if data was not ready in time
 */
int paa5100je_get_motion_seq(const paa5100je_t *dev, int16_t *x, int16_t *y);

#ifdef __cplusplus
}
#endif

/** @} */
