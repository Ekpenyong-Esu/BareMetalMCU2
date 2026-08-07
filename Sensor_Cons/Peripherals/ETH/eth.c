/**
 * @file eth.c
 * @brief Ethernet initialization, lifecycle and MAC configuration
 */

#include "eth_core.h"
#include "eth_buffers.h"
#include "eth_irq.h"
#include "log.h"
#include <string.h>

/**
 * @brief   Validate a requested configuration
 */
static bool ETH_ValidateConfig(const ETH_Config_t *config)
{
    if (config == NULL) {
        return false;
    }

    if ((config->speed != ETH_SPEED_10M) && (config->speed != ETH_SPEED_100M)) {
        return false;
    }

    if ((config->duplexMode != ETH_FULLDUPLEX_MODE) &&
        (config->duplexMode != ETH_HALFDUPLEX_MODE)) {
        return false;
    }

    if ((config->mediaInterface != ETH_MEDIA_INTERFACE_MII) &&
        (config->mediaInterface != ETH_MEDIA_INTERFACE_RMII)) {
        return false;
    }

    /* A multicast source address is never legal as a station address. */
    if ((config->macAddr[0] & 0x01U) != 0U) {
        return false;
    }

    return true;
}

HAL_StatusTypeDef ETH_Init(ETH_Handle_t *handle, const ETH_Config_t *config)
{
    ETH_MACConfigTypeDef macConfig;

    if (handle == NULL) {
        return HAL_ERROR;
    }

    if (!ETH_ValidateConfig(config)) {
        log_error("ETH: invalid configuration");
        return HAL_ERROR;
    }

    memset(handle, 0, sizeof(ETH_Handle_t));
    handle->config = *config;

    handle->heth.Instance = ETH;
    handle->heth.Init.MACAddr = handle->config.macAddr;
    handle->heth.Init.MediaInterface = config->mediaInterface;
    ETH_Buffers_Attach(&handle->heth);
    ETH_Buffers_Reset();

    if (HAL_ETH_Init(&handle->heth) != HAL_OK) {
        log_error("ETH: HAL_ETH_Init failed");
        return HAL_ERROR;
    }

    /* Apply the requested speed and duplex on top of the reset defaults. */
    if (HAL_ETH_GetMACConfig(&handle->heth, &macConfig) != HAL_OK) {
        (void)HAL_ETH_DeInit(&handle->heth);
        log_error("ETH: unable to read MAC configuration");
        return HAL_ERROR;
    }

    macConfig.Speed = config->speed;
    macConfig.DuplexMode = config->duplexMode;

    if (HAL_ETH_SetMACConfig(&handle->heth, &macConfig) != HAL_OK) {
        (void)HAL_ETH_DeInit(&handle->heth);
        log_error("ETH: unable to apply MAC configuration");
        return HAL_ERROR;
    }

    handle->initialized = true;
    log_info("ETH: initialized");

    return HAL_OK;
}

HAL_StatusTypeDef ETH_DeInit(ETH_Handle_t *handle)
{
    if ((handle == NULL) || !handle->initialized) {
        return HAL_ERROR;
    }

    if (HAL_ETH_DeInit(&handle->heth) != HAL_OK) {
        return HAL_ERROR;
    }

    handle->initialized = false;
    ETH_Buffers_Reset();

    return HAL_OK;
}

HAL_StatusTypeDef ETH_Start(ETH_Handle_t *handle)
{
    if ((handle == NULL) || !handle->initialized) {
        return HAL_ERROR;
    }

    if (ETH_IsInterruptModeEnabled()) {
        return HAL_ETH_Start_IT(&handle->heth);
    }

    return HAL_ETH_Start(&handle->heth);
}

HAL_StatusTypeDef ETH_Stop(ETH_Handle_t *handle)
{
    if ((handle == NULL) || !handle->initialized) {
        return HAL_ERROR;
    }

    return HAL_ETH_Stop(&handle->heth);
}

bool ETH_IsReady(const ETH_Handle_t *handle)
{
    if ((handle == NULL) || !handle->initialized) {
        return false;
    }

    return (handle->heth.gState == HAL_ETH_STATE_STARTED);
}

bool ETH_IsInitialized(const ETH_Handle_t *handle)
{
    return ((handle != NULL) && handle->initialized);
}

uint32_t ETH_GetConfiguredSpeed(const ETH_Handle_t *handle)
{
    if ((handle == NULL) || !handle->initialized) {
        return 0U;
    }

    return handle->config.speed;
}

uint32_t ETH_GetConfiguredDuplex(const ETH_Handle_t *handle)
{
    if ((handle == NULL) || !handle->initialized) {
        return 0U;
    }

    return handle->config.duplexMode;
}

HAL_StatusTypeDef ETH_SetMACAddress(ETH_Handle_t *handle, const uint8_t *macAddr)
{
    uint32_t high;
    uint32_t low;

    if ((handle == NULL) || (macAddr == NULL) || !handle->initialized) {
        return HAL_ERROR;
    }

    if ((macAddr[0] & 0x01U) != 0U) {
        log_error("ETH: multicast address rejected as station address");
        return HAL_ERROR;
    }

    memcpy(handle->config.macAddr, macAddr, ETH_ADDR_LEN);

    high = ((uint32_t)macAddr[5] << 8) | (uint32_t)macAddr[4];
    low = ((uint32_t)macAddr[3] << 24) | ((uint32_t)macAddr[2] << 16) |
          ((uint32_t)macAddr[1] << 8) | (uint32_t)macAddr[0];

    handle->heth.Instance->MACA0HR = high;
    handle->heth.Instance->MACA0LR = low;

    return HAL_OK;
}

HAL_StatusTypeDef ETH_GetMACAddress(const ETH_Handle_t *handle, uint8_t *macAddr)
{
    if ((handle == NULL) || (macAddr == NULL) || !handle->initialized) {
        return HAL_ERROR;
    }

    memcpy(macAddr, handle->config.macAddr, ETH_ADDR_LEN);

    return HAL_OK;
}
