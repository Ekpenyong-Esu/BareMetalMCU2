/**
 * @file eth_core.h
 * @brief Ethernet initialization, lifecycle and MAC configuration
 */

#ifndef ETH_CORE_H
#define ETH_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "eth_types.h"

/**
 * @brief   Initialize the Ethernet peripheral
 * @param   handle Driver handle to populate
 * @param   config Requested configuration
 * @retval  HAL_StatusTypeDef HAL_OK on success
 * @note    HAL_ETH_MspInit() must supply the RMII/MII pins and clocks. The
 *          STM32F429I-DISC1 carries no PHY, so this driver is only usable on
 *          a board that adds one.
 */
HAL_StatusTypeDef ETH_Init(ETH_Handle_t *handle, const ETH_Config_t *config);

/**
 * @brief   Deinitialize the Ethernet peripheral
 * @param   handle Driver handle
 * @retval  HAL_StatusTypeDef HAL_OK on success
 */
HAL_StatusTypeDef ETH_DeInit(ETH_Handle_t *handle);

/**
 * @brief   Start transmission and reception
 * @param   handle Driver handle
 * @retval  HAL_StatusTypeDef HAL_OK on success
 */
HAL_StatusTypeDef ETH_Start(ETH_Handle_t *handle);

/**
 * @brief   Stop transmission and reception
 * @param   handle Driver handle
 * @retval  HAL_StatusTypeDef HAL_OK on success
 */
HAL_StatusTypeDef ETH_Stop(ETH_Handle_t *handle);

/**
 * @brief   Report whether the peripheral is initialized and started
 * @param   handle Driver handle
 * @retval  bool true when ready to carry traffic
 */
bool ETH_IsReady(const ETH_Handle_t *handle);

/**
 * @brief   Report whether the peripheral has been initialized
 * @param   handle Driver handle
 * @retval  bool true when initialized
 */
bool ETH_IsInitialized(const ETH_Handle_t *handle);

/**
 * @brief   Read back the configured link speed
 * @param   handle Driver handle
 * @retval  uint32_t ETH_SPEED_10M or ETH_SPEED_100M, 0 when not initialized
 */
uint32_t ETH_GetConfiguredSpeed(const ETH_Handle_t *handle);

/**
 * @brief   Read back the configured duplex mode
 * @param   handle Driver handle
 * @retval  uint32_t ETH_FULLDUPLEX_MODE or ETH_HALFDUPLEX_MODE, 0 when not initialized
 */
uint32_t ETH_GetConfiguredDuplex(const ETH_Handle_t *handle);

/**
 * @brief   Program a new station MAC address
 * @param   handle Driver handle
 * @param   macAddr Six byte address
 * @retval  HAL_StatusTypeDef HAL_OK on success
 */
HAL_StatusTypeDef ETH_SetMACAddress(ETH_Handle_t *handle, const uint8_t *macAddr);

/**
 * @brief   Read the station MAC address in force
 * @param   handle Driver handle
 * @param   macAddr Destination for six bytes
 * @retval  HAL_StatusTypeDef HAL_OK on success
 */
HAL_StatusTypeDef ETH_GetMACAddress(const ETH_Handle_t *handle, uint8_t *macAddr);

#ifdef __cplusplus
}
#endif

#endif /* ETH_CORE_H */
