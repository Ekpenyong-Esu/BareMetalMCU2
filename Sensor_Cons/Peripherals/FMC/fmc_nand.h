/**
 * @file fmc_nand.h
 * @brief NAND Flash bring-up and page access
 */

#ifndef FMC_NAND_H
#define FMC_NAND_H

#include "fmc_types.h"

/**
 * @brief Initialize FMC driver with NAND Flash configuration
 */
HAL_StatusTypeDef FMC_Driver_NAND_Init(FMC_Driver_Handle_t *handle,
                                       const FMC_Driver_NAND_Config_t *config);

/**
 * @brief Write whole pages to NAND Flash
 * @param address Byte offset from the start of the device, page aligned
 * @param size    Byte count, a whole multiple of the page size
 */
HAL_StatusTypeDef FMC_Driver_NAND_Write(FMC_Driver_Handle_t *handle, uint32_t address,
                                        const uint8_t *data, uint32_t size);

/**
 * @brief Read whole pages from NAND Flash
 * @param address Byte offset from the start of the device, page aligned
 * @param size    Byte count, a whole multiple of the page size
 */
HAL_StatusTypeDef FMC_Driver_NAND_Read(FMC_Driver_Handle_t *handle, uint32_t address, uint8_t *data,
                                       uint32_t size);

/**
 * @brief Erase the NAND block containing a byte offset
 */
HAL_StatusTypeDef FMC_Driver_NAND_EraseBlock(FMC_Driver_Handle_t *handle, uint32_t address);

#endif /* FMC_NAND_H */
