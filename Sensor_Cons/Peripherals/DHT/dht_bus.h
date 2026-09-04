/**
 ******************************************************************************
 * @file    dht_bus.h
 * @brief   DHT one-wire transport - internal to the DHT driver
 * @details Owns the data line: its direction, the start pulse, and decoding
 *          the 40 pulse widths into raw bytes. It does not know what those
 *          bytes mean. Not part of dht.h.
 ******************************************************************************
 */

#ifndef DHT_BUS_H
#define DHT_BUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dht_types.h"

/** Host start pulse: DHT11 needs >= 18 ms low, DHT22 only >= 1 ms */
#define DHT11_START_PULSE_MS 18U
#define DHT22_START_PULSE_MS 2U

/**
 * @brief   Drive the line high as an output, which is the bus idle state.
 * @note    Also used to park the line so the sensor can see the next start.
 */
void DHT_Bus_Release(GPIO_TypeDef *port, uint16_t pin);

/**
 * @brief   Send the start pulse and clock in one 40-bit frame.
 * @param   startPulseMs How long to hold the line low to wake the sensor.
 * @param   frame        Destination, DHT_FRAME_BYTES long.
 * @retval  HAL_OK on a complete frame, HAL_TIMEOUT if an edge never came.
 * @note    Runs with interrupts masked; the frame takes about 5 ms.
 */
HAL_StatusTypeDef DHT_Bus_ReadFrame(GPIO_TypeDef *port, uint16_t pin, uint32_t startPulseMs,
                                    uint8_t *frame);

#ifdef __cplusplus
}
#endif

#endif /* DHT_BUS_H */
