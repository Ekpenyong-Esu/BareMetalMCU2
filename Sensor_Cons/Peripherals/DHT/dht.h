/**
 ******************************************************************************
 * @file    dht.h
 * @brief   DHT11 / DHT22 temperature and humidity sensor driver
 * @details Aggregator header: include this to get the whole public API. One
 *          wire is used for both sending the start signal and reading the
 *          40-bit reply. The sensor is slow, so the driver enforces a gap
 *          between reads (1 s for DHT11, 2 s for DHT22).
 *
 * How it works (in simple words):
 * - The MCU pulls the data line low to wake the sensor.
 * - The sensor replies with 40 bits: humidity + temperature + checksum.
 * - Each bit is coded by how long the line stays high (short = 0, long = 1).
 * - The driver checks the checksum and saves humidity and temperature.
 *
 * Layering, top to bottom (each layer only calls the one below):
 * - dht_core.h    lifecycle, read policy, last-sample access  (public)
 * - dht_decode.h  checksum and raw bytes to degrees C / %RH   (public, pure)
 * - dht_bus.h     the one-wire line and the 40 pulse widths   (internal)
 * - dht_timing.h  microsecond time base from the DWT counter  (internal)
 * - dht_types.h   shared vocabulary, no behaviour
 */

#ifndef DHT_H
#define DHT_H

#include "dht_core.h"
#include "dht_decode.h"
#include "dht_types.h"

#endif /* DHT_H */
