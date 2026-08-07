/**
 * @file fmc_core.h
 * @brief Handle lifetime and error reporting shared by every FMC memory type
 */

#ifndef FMC_CORE_H
#define FMC_CORE_H

#include "fmc_types.h"

/**
 * @brief Deinitialize the configured memory and release the handle
 */
HAL_StatusTypeDef FMC_Driver_DeInit(FMC_Driver_Handle_t *handle);

/**
 * @brief Last error recorded on a handle
 * @retval FMC_DRIVER_ERROR_PARAM if the handle is NULL
 */
uint32_t FMC_Driver_GetError(const FMC_Driver_Handle_t *handle);

/**
 * @brief Whether a handle is initialized
 */
bool FMC_Driver_IsInitialized(const FMC_Driver_Handle_t *handle);

/**
 * @brief Base address of the memory window described by a handle, 0 if none
 */
uint32_t FMC_Driver_GetBaseAddress(const FMC_Driver_Handle_t *handle);

/**
 * @brief Size in bytes of the memory window described by a handle, 0 if none
 */
uint32_t FMC_Driver_GetSize(const FMC_Driver_Handle_t *handle);

#endif /* FMC_CORE_H */
