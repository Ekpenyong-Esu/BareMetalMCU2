/**
 ******************************************************************************
 * @file    crc_sw.h
 * @brief   Software CRC implementation
 ******************************************************************************
 */

#ifndef CRC_SW_H
#define CRC_SW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "crc_types.h"

/**
 * @brief  Run a byte buffer through the bit-wise CRC, continuing from @p seed.
 * @param  config Configuration; only @c polynomial and @c input_reverse are used.
 * @param  seed   Running value, @c config->init_value for the first block.
 * @return The raw running CRC. Pass it to CRC_SoftwareFinalize() when done.
 * @note   Returns @p seed unchanged if @p config or @p data is NULL.
 */
uint32_t CRC_SoftwareUpdate(const CRC_Config *config, uint32_t seed, const uint8_t *data,
                            uint32_t size);

/**
 * @brief  Word-wide counterpart of CRC_SoftwareUpdate().
 * @param  size Number of 32-bit words, not bytes.
 */
uint32_t CRC_SoftwareUpdate32(const CRC_Config *config, uint32_t seed, const uint32_t *data,
                              uint32_t size);

/**
 * @brief  Apply the output reflection to a running CRC.
 * @note   Reflection must only be applied once, after the last block, which is
 *         why it is not part of CRC_SoftwareUpdate().
 */
uint32_t CRC_SoftwareFinalize(const CRC_Config *config, uint32_t crc);

#ifdef __cplusplus
}
#endif

#endif /* CRC_SW_H */
