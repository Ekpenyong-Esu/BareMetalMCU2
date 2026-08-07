/**
 * @file fmc.c
 * @brief Handle lifetime and error reporting shared by every FMC memory type
 */

#include "fmc_core.h"
#include "log.h"
#include <string.h>

HAL_StatusTypeDef FMC_Driver_DeInit(FMC_Driver_Handle_t *handle)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint32_t memoryType;

    if (handle == NULL) {
        return HAL_ERROR;
    }

    if (!handle->initialized) {
        return HAL_OK;
    }

    memoryType = handle->memoryType;

    /* Every branch is folded into one status so a later success cannot hide
       an earlier failure. */
    if ((memoryType & FMC_DRIVER_MEMORY_SDRAM) != 0U) {
        if (HAL_SDRAM_DeInit(&handle->hsdram) != HAL_OK) {
            status = HAL_ERROR;
        }
    }

    if ((memoryType & FMC_DRIVER_MEMORY_NOR) != 0U) {
        if (HAL_SRAM_DeInit(&handle->hsram) != HAL_OK) {
            status = HAL_ERROR;
        }
    }

    if ((memoryType & FMC_DRIVER_MEMORY_NAND) != 0U) {
        if (HAL_NAND_DeInit(&handle->hnand) != HAL_OK) {
            status = HAL_ERROR;
        }
    }

    if (status != HAL_OK) {
        /* Leave the handle usable so the caller can still read the error. */
        handle->errorCode = FMC_DRIVER_ERROR_OPERATION;
        log_error("FMC: deinit failed for memory type 0x%02lX", (unsigned long)memoryType);
        return status;
    }

    memset(handle, 0, sizeof(FMC_Driver_Handle_t));
    handle->memoryType = FMC_DRIVER_MEMORY_NONE;
    handle->errorCode = FMC_DRIVER_ERROR_NONE;
    handle->initialized = false;

    return HAL_OK;
}

uint32_t FMC_Driver_GetError(const FMC_Driver_Handle_t *handle)
{
    /* A missing handle is a caller mistake, not an initialization failure. */
    if (handle == NULL) {
        return FMC_DRIVER_ERROR_PARAM;
    }

    return handle->errorCode;
}

bool FMC_Driver_IsInitialized(const FMC_Driver_Handle_t *handle)
{
    return (handle != NULL) && handle->initialized;
}

uint32_t FMC_Driver_GetBaseAddress(const FMC_Driver_Handle_t *handle)
{
    if (handle == NULL || !handle->initialized) {
        return 0U;
    }

    return handle->baseAddress;
}

uint32_t FMC_Driver_GetSize(const FMC_Driver_Handle_t *handle)
{
    if (handle == NULL || !handle->initialized) {
        return 0U;
    }

    return handle->deviceSize;
}
