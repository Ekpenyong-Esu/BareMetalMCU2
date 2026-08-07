/**
 * @file fmc_nand.c
 * @brief NAND Flash bring-up and page access
 */

#include "fmc_nand.h"
#include "log.h"
#include <string.h>

/**
 * @brief Translate a byte offset into the page/block/plane triplet HAL expects
 * @note  The geometry comes from the handle, so a device with more than 256
 *        blocks is addressed correctly instead of wrapping at 8 bits.
 */
static bool FMC_Driver_NAND_Address(const FMC_Driver_Handle_t *handle, uint32_t address,
                                    NAND_AddressTypeDef *nandAddress)
{
    const NAND_DeviceConfigTypeDef *geometry = &handle->hnand.Config;
    uint32_t page;
    uint32_t block;
    uint32_t plane;

    if (geometry->PageSize == 0U || geometry->BlockSize == 0U || geometry->PlaneSize == 0U) {
        return false;
    }

    if ((address % geometry->PageSize) != 0U) {
        return false;
    }

    page = address / geometry->PageSize;
    block = page / geometry->BlockSize;
    plane = block / geometry->PlaneSize;

    if (block >= geometry->BlockNbr || plane >= geometry->PlaneNbr) {
        return false;
    }

    nandAddress->Page = (uint16_t)(page % geometry->BlockSize);
    nandAddress->Block = (uint16_t)(block % geometry->PlaneSize);
    nandAddress->Plane = (uint16_t)plane;

    return true;
}

HAL_StatusTypeDef FMC_Driver_NAND_Init(FMC_Driver_Handle_t *handle,
                                       const FMC_Driver_NAND_Config_t *config)
{
    FMC_NAND_PCC_TimingTypeDef comSpaceTiming = {0};
    FMC_NAND_PCC_TimingTypeDef attSpaceTiming = {0};
    NAND_DeviceConfigTypeDef deviceConfig = {0};

    if (handle == NULL || config == NULL) {
        return HAL_ERROR;
    }

    if (config->pageSize == 0U || config->blockSize == 0U ||
        config->planeSize == 0U || config->planeNbr == 0U || config->blockNbr == 0U) {
        log_error("FMC: NAND geometry is incomplete");
        return HAL_ERROR;
    }

    memset(handle, 0, sizeof(FMC_Driver_Handle_t));

    handle->hnand.Instance = FMC_NAND_DEVICE;
    handle->hnand.Init.NandBank = config->bank;
    handle->hnand.Init.Waitfeature = config->waitFeature;
    handle->hnand.Init.MemoryDataWidth = config->memoryDataWidth;
    handle->hnand.Init.EccComputation = config->eccComputation;
    handle->hnand.Init.ECCPageSize = config->eccPageSize;
    handle->hnand.Init.TCLRSetupTime = config->tclrSetupTime;
    handle->hnand.Init.TARSetupTime = config->tarSetupTime;

    comSpaceTiming.SetupTime = config->tset;
    comSpaceTiming.WaitSetupTime = config->twait;
    comSpaceTiming.HoldSetupTime = config->thold;
    comSpaceTiming.HiZSetupTime = config->thiz;

    attSpaceTiming = comSpaceTiming;

    if (HAL_NAND_Init(&handle->hnand, &comSpaceTiming, &attSpaceTiming) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_INIT;
        return HAL_ERROR;
    }

    /* Without this the geometry stays zero and every page transfer moves no data. */
    deviceConfig.PageSize = config->pageSize;
    deviceConfig.SpareAreaSize = config->spareAreaSize;
    deviceConfig.BlockSize = config->blockSize;
    deviceConfig.BlockNbr = config->blockNbr;
    deviceConfig.PlaneNbr = config->planeNbr;
    deviceConfig.PlaneSize = config->planeSize;
    deviceConfig.ExtraCommandEnable = config->extraCommandEnable ? ENABLE : DISABLE;

    if (HAL_NAND_ConfigDevice(&handle->hnand, &deviceConfig) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_CONFIG;
        return HAL_ERROR;
    }

    handle->memoryType = FMC_DRIVER_MEMORY_NAND;
    handle->baseAddress = 0U;
    handle->deviceSize = config->pageSize * config->blockSize * config->blockNbr;
    handle->initialized = true;
    handle->errorCode = FMC_DRIVER_ERROR_NONE;

    return HAL_OK;
}

/**
 * @brief Common guard for the page transfers
 * @param[out] pageCount Number of whole pages the transfer covers
 */
static HAL_StatusTypeDef FMC_Driver_NAND_CheckTransfer(FMC_Driver_Handle_t *handle,
                                                       uint32_t address, const void *data,
                                                       uint32_t size, uint32_t *pageCount)
{
    uint32_t pageSize;

    if (handle == NULL || data == NULL || !handle->initialized ||
        handle->memoryType != FMC_DRIVER_MEMORY_NAND) {
        return HAL_ERROR;
    }

    pageSize = handle->hnand.Config.PageSize;
    if (pageSize == 0U || size == 0U || (size % pageSize) != 0U) {
        handle->errorCode = FMC_DRIVER_ERROR_PARAM;
        return HAL_ERROR;
    }

    if (size > handle->deviceSize || address > handle->deviceSize - size) {
        handle->errorCode = FMC_DRIVER_ERROR_PARAM;
        return HAL_ERROR;
    }

    *pageCount = size / pageSize;

    return HAL_OK;
}

HAL_StatusTypeDef FMC_Driver_NAND_Write(FMC_Driver_Handle_t *handle, uint32_t address,
                                        const uint8_t *data, uint32_t size)
{
    NAND_AddressTypeDef nandAddress = {0};
    uint32_t pageCount = 0U;
    HAL_StatusTypeDef status;

    status = FMC_Driver_NAND_CheckTransfer(handle, address, data, size, &pageCount);
    if (status != HAL_OK) {
        return status;
    }

    if (!FMC_Driver_NAND_Address(handle, address, &nandAddress)) {
        handle->errorCode = FMC_DRIVER_ERROR_PARAM;
        return HAL_ERROR;
    }

    /* The whole run is passed to HAL, which walks the pages itself. */
    if (HAL_NAND_Write_Page_8b(&handle->hnand, &nandAddress,
                               (uint8_t *)(uintptr_t)data, pageCount) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_OPERATION;
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef FMC_Driver_NAND_Read(FMC_Driver_Handle_t *handle, uint32_t address,
                                       uint8_t *data, uint32_t size)
{
    NAND_AddressTypeDef nandAddress = {0};
    uint32_t pageCount = 0U;
    HAL_StatusTypeDef status;

    status = FMC_Driver_NAND_CheckTransfer(handle, address, data, size, &pageCount);
    if (status != HAL_OK) {
        return status;
    }

    if (!FMC_Driver_NAND_Address(handle, address, &nandAddress)) {
        handle->errorCode = FMC_DRIVER_ERROR_PARAM;
        return HAL_ERROR;
    }

    if (HAL_NAND_Read_Page_8b(&handle->hnand, &nandAddress, data, pageCount) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_OPERATION;
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef FMC_Driver_NAND_EraseBlock(FMC_Driver_Handle_t *handle, uint32_t address)
{
    NAND_AddressTypeDef nandAddress = {0};
    uint32_t blockBytes;

    if (handle == NULL || !handle->initialized ||
        handle->memoryType != FMC_DRIVER_MEMORY_NAND) {
        return HAL_ERROR;
    }

    blockBytes = handle->hnand.Config.PageSize * handle->hnand.Config.BlockSize;
    if (blockBytes == 0U || address >= handle->deviceSize) {
        handle->errorCode = FMC_DRIVER_ERROR_PARAM;
        return HAL_ERROR;
    }

    /* Erase granularity is a block, so round down to the block the caller named. */
    if (!FMC_Driver_NAND_Address(handle, address - (address % blockBytes), &nandAddress)) {
        handle->errorCode = FMC_DRIVER_ERROR_PARAM;
        return HAL_ERROR;
    }

    if (HAL_NAND_Erase_Block(&handle->hnand, &nandAddress) != HAL_OK) {
        handle->errorCode = FMC_DRIVER_ERROR_OPERATION;
        return HAL_ERROR;
    }

    return HAL_OK;
}
