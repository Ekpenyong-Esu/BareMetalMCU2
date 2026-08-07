/**
  ******************************************************************************
  * @file    can_baudrate.c
  * @brief   Baud-rate timing derivation for the CAN peripheral
  ******************************************************************************
  */

#include "can_baudrate.h"
#include "stm32f4xx_hal.h"

/**
 * 1 sync quantum + BS1 + BS2 per bit. The sample point lands at 12/14 (85.7%),
 * inside the 75-90% window CiA recommends, and 14 tq divides the APB1 clock
 * exactly for every preset in can_types.h.
 */
#define CAN_TQ_PER_BIT              14U
#define CAN_BS1_SETTING             CAN_BS1_11TQ
#define CAN_BS2_SETTING             CAN_BS2_2TQ
#define CAN_SJW_SETTING             CAN_SJW_1TQ

#define CAN_PRESCALER_MIN           1U
#define CAN_PRESCALER_MAX           1024U

HAL_StatusTypeDef CAN_ApplyBaudRate(CAN_HandleTypeDef *hcan, const CAN_Config *config)
{
    uint32_t pclk1;
    uint32_t divider;

    if (hcan == NULL || config == NULL) {
        return HAL_ERROR;
    }

    /* Custom timing: the caller supplies raw register values. */
    if (config->baud_rate == 0U) {
        if (config->prescaler < CAN_PRESCALER_MIN || config->prescaler > CAN_PRESCALER_MAX) {
            return HAL_ERROR;
        }

        hcan->Init.Prescaler     = config->prescaler;
        hcan->Init.SyncJumpWidth = config->sync_jump_width;
        hcan->Init.TimeSeg1      = config->time_segment_1;
        hcan->Init.TimeSeg2      = config->time_segment_2;

        return HAL_OK;
    }

    /* Derive the prescaler from the live APB1 frequency rather than a table
       baked for one clock setup: a table silently produces the wrong bit rate
       when the clock tree changes. */
    pclk1 = HAL_RCC_GetPCLK1Freq();
    divider = config->baud_rate * CAN_TQ_PER_BIT;

    if (divider == 0U || (pclk1 % divider) != 0U) {
        return HAL_ERROR;
    }

    hcan->Init.Prescaler = pclk1 / divider;

    if (hcan->Init.Prescaler < CAN_PRESCALER_MIN ||
        hcan->Init.Prescaler > CAN_PRESCALER_MAX) {
        return HAL_ERROR;
    }

    hcan->Init.SyncJumpWidth = CAN_SJW_SETTING;
    hcan->Init.TimeSeg1      = CAN_BS1_SETTING;
    hcan->Init.TimeSeg2      = CAN_BS2_SETTING;

    return HAL_OK;
}
