/**
 * @file fmc_sdram.h
 * @brief SDRAM bring-up and memory-mapped access
 */

#ifndef FMC_SDRAM_H
#define FMC_SDRAM_H

#include "fmc_types.h"

/**
 * @brief Initialize FMC driver with SDRAM configuration
 */
HAL_StatusTypeDef FMC_Driver_SDRAM_Init(FMC_Driver_Handle_t *handle,
                                        const FMC_Driver_SDRAM_Config_t *config);

/**
 * @brief Default SDRAM configuration for the on-board IS42S16400J (bank 2)
 */
FMC_Driver_SDRAM_Config_t FMC_Driver_SDRAM_GetDefaultConfig(void);

/**
 * @brief Size in bytes of the window described by an SDRAM configuration
 */
uint32_t FMC_Driver_SDRAM_GetSize(const FMC_Driver_SDRAM_Config_t *config);

/**
 * @brief Base address of the window described by an SDRAM configuration
 */
uint32_t FMC_Driver_SDRAM_GetBase(const FMC_Driver_SDRAM_Config_t *config);

/**
 * @brief Auto-refresh counter for a configuration, 0 if it cannot be met
 */
uint32_t FMC_Driver_SDRAM_GetRefreshCount(const FMC_Driver_SDRAM_Config_t *config);

/**
 * @brief Write data to SDRAM
 */
HAL_StatusTypeDef FMC_Driver_SDRAM_Write(FMC_Driver_Handle_t *handle, uint32_t address,
                                         const uint8_t *data, uint32_t size);

/**
 * @brief Read data from SDRAM
 */
HAL_StatusTypeDef FMC_Driver_SDRAM_Read(FMC_Driver_Handle_t *handle, uint32_t address,
                                        uint8_t *data, uint32_t size);

/**
 * @brief Destructive SDRAM pattern test
 * @note  Overwrites the tested region.
 */
bool FMC_Driver_SDRAM_Test(FMC_Driver_Handle_t *handle, uint32_t startAddr, uint32_t size);

#endif /* FMC_SDRAM_H */
