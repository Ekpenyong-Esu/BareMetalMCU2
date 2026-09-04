/**
 * @file fmc_nor.h
 * @brief NOR Flash bring-up and access
 */

#ifndef FMC_NOR_H
#define FMC_NOR_H

#include "fmc_types.h"

/**
 * @brief Initialize FMC driver with NOR Flash configuration
 */
HAL_StatusTypeDef FMC_Driver_NOR_Init(FMC_Driver_Handle_t *handle,
                                      const FMC_Driver_NOR_Config_t *config);

/**
 * @brief Base address of the NOR/PSRAM sub-bank selected by a configuration
 */
uint32_t FMC_Driver_NOR_GetBase(const FMC_Driver_NOR_Config_t *config);

/**
 * @brief Write data to NOR Flash
 */
HAL_StatusTypeDef FMC_Driver_NOR_Write(FMC_Driver_Handle_t *handle, uint32_t address,
                                       const uint8_t *data, uint32_t size);

/**
 * @brief Read data from NOR Flash
 */
HAL_StatusTypeDef FMC_Driver_NOR_Read(FMC_Driver_Handle_t *handle, uint32_t address, uint8_t *data,
                                      uint32_t size);

/**
 * @brief Erase the NOR Flash sector containing an address
 */
HAL_StatusTypeDef FMC_Driver_NOR_EraseSector(FMC_Driver_Handle_t *handle, uint32_t address);

#endif /* FMC_NOR_H */
