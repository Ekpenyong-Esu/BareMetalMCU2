/**
 * @file ir_transmit.c
 * @brief IR transmit path: protocol encoding and blocking pulse emission
 */

/* Includes ------------------------------------------------------------------*/
#include "ir_transmit.h"
#include "ir_core.h"
#include "ir_timing.h"

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef IR_BeginTransmit(IR_Handle_t *handle);
static HAL_StatusTypeDef IR_StartTransmission(IR_Handle_t *handle);
static void IR_AddPulseToTxBuffer(IR_Handle_t *handle, uint16_t mark, uint16_t space);
static void IR_EmitPulse(IR_Handle_t *handle, const IR_Pulse_t *pulse);

/* Public functions ----------------------------------------------------------*/

HAL_StatusTypeDef IR_TransmitNEC(IR_Handle_t *handle, uint8_t address, uint8_t command)
{
    uint32_t data;

    if (IR_BeginTransmit(handle) != HAL_OK)
    {
        return HAL_ERROR;
    }

    IR_AddPulseToTxBuffer(handle, IR_NEC_HEADER_MARK, IR_NEC_HEADER_SPACE);

    /* Each complement must be truncated to 8 bits before it is widened: ~address
       promotes to int first and would otherwise set every higher bit. */
    data = ((uint32_t)address << 24) |
           ((uint32_t)(uint8_t)~address << 16) |
           ((uint32_t)command << 8) |
           (uint32_t)(uint8_t)~command;

    for (uint16_t i = 0; i < IR_NEC_DATA_BITS; i++)
    {
        const uint32_t mask = 1UL << (IR_NEC_DATA_BITS - 1U - i);   /* MSB first */
        const uint16_t space = ((data & mask) != 0U) ? IR_NEC_BIT_1_SPACE : IR_NEC_BIT_0_SPACE;

        IR_AddPulseToTxBuffer(handle, IR_NEC_BIT_MARK, space);
    }

    IR_AddPulseToTxBuffer(handle, IR_NEC_STOP_BIT, 0);

    return IR_StartTransmission(handle);
}

HAL_StatusTypeDef IR_TransmitRC5(IR_Handle_t *handle, uint8_t address, uint8_t command)
{
    uint16_t data;

    if (IR_BeginTransmit(handle) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (address > IR_RC5_MAX_ADDRESS || command > IR_RC5_MAX_COMMAND)
    {
        handle->errorCode = IR_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    data = (uint16_t)((IR_RC5_START_BITS << 12) | ((uint16_t)address << 6) | command);

    for (uint16_t i = 0; i < IR_RC5_DATA_BITS; i++)
    {
        const uint16_t mask = (uint16_t)(1U << (IR_RC5_DATA_BITS - 1U - i));

        if ((data & mask) != 0U)
        {
            /* Bit 1 is a space followed by a mark */
            IR_AddPulseToTxBuffer(handle, 0, IR_RC5_HALF_BIT);
            IR_AddPulseToTxBuffer(handle, IR_RC5_HALF_BIT, 0);
        }
        else
        {
            /* Bit 0 is a mark followed by a space */
            IR_AddPulseToTxBuffer(handle, IR_RC5_HALF_BIT, 0);
            IR_AddPulseToTxBuffer(handle, 0, IR_RC5_HALF_BIT);
        }
    }

    return IR_StartTransmission(handle);
}

HAL_StatusTypeDef IR_TransmitSIRC(IR_Handle_t *handle, uint8_t address, uint8_t command)
{
    uint16_t data;

    if (IR_BeginTransmit(handle) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (address > IR_SIRC_MAX_ADDRESS || command > IR_SIRC_MAX_COMMAND)
    {
        handle->errorCode = IR_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    IR_AddPulseToTxBuffer(handle, IR_SIRC_HEADER_MARK, IR_SIRC_BIT_0_SPACE);

    data = (uint16_t)(((uint16_t)address << 7) | command);

    for (uint16_t i = 0; i < IR_SIRC_DATA_BITS; i++)
    {
        const uint16_t mask = (uint16_t)(1U << i);      /* LSB first */
        const uint16_t space = ((data & mask) != 0U) ? IR_SIRC_BIT_1_SPACE : IR_SIRC_BIT_0_SPACE;

        IR_AddPulseToTxBuffer(handle, IR_SIRC_BIT_MARK, space);
    }

    return IR_StartTransmission(handle);
}

HAL_StatusTypeDef IR_TransmitCustom(IR_Handle_t *handle, const IR_Pulse_t *pulses, uint16_t count)
{
    if (IR_BeginTransmit(handle) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (pulses == NULL || count == 0U)
    {
        handle->errorCode = IR_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    if (count > IR_TX_BUFFER_SIZE)
    {
        handle->errorCode = IR_ERROR_BUFFER_OVERFLOW;
        return HAL_ERROR;
    }

    for (uint16_t i = 0; i < count; i++)
    {
        handle->txBuffer[i] = pulses[i];
    }

    handle->txCount = count;

    return IR_StartTransmission(handle);
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Validate the handle and take ownership of the transmit buffer
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef IR_BeginTransmit(IR_Handle_t *handle)
{
    if (IR_ValidateHandle(handle) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (handle->state == IR_STATE_TRANSMITTING)
    {
        handle->errorCode = IR_ERROR_TX_BUSY;
        return HAL_ERROR;
    }

    handle->txCount = 0;
    handle->txIndex = 0;

    return HAL_OK;
}

/**
 * @brief Emit the prepared pulse train
 * @note  Blocking: the mark and space durations are held with a busy-wait.
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef IR_StartTransmission(IR_Handle_t *handle)
{
    if (handle->txCount == 0U)
    {
        handle->errorCode = IR_ERROR_BUFFER_OVERFLOW;
        return HAL_ERROR;
    }

    handle->state = IR_STATE_TRANSMITTING;

    for (handle->txIndex = 0; handle->txIndex < handle->txCount; handle->txIndex++)
    {
        IR_EmitPulse(handle, &handle->txBuffer[handle->txIndex]);
    }

    handle->state = IR_STATE_IDLE;

    IR_NotifyEvent(handle, IR_EVENT_FRAME_TRANSMITTED, NULL);

    return HAL_OK;
}

/**
 * @brief Append one mark/space pair to the transmit buffer
 * @note  Silently drops pulses once the buffer is full; the callers all build
 *        frames far shorter than IR_TX_BUFFER_SIZE.
 * @param handle: Pointer to IR handle structure
 * @param mark: Mark time in microseconds
 * @param space: Space time in microseconds
 * @return void
 */
static void IR_AddPulseToTxBuffer(IR_Handle_t *handle, uint16_t mark, uint16_t space)
{
    if (handle->txCount >= IR_TX_BUFFER_SIZE)
    {
        handle->errorCode = IR_ERROR_BUFFER_OVERFLOW;
        return;
    }

    handle->txBuffer[handle->txCount].mark = mark;
    handle->txBuffer[handle->txCount].space = space;
    handle->txCount++;
}

/**
 * @brief Emit one mark (carrier on) followed by one space (carrier off)
 * @param handle: Pointer to IR handle structure
 * @param pulse: Pulse to emit
 * @return void
 */
static void IR_EmitPulse(IR_Handle_t *handle, const IR_Pulse_t *pulse)
{
    if (pulse->mark != 0U)
    {
        (void)HAL_TIM_PWM_Start(handle->htimCarrier, handle->txChannel);
        IR_DelayUs(pulse->mark);
        (void)HAL_TIM_PWM_Stop(handle->htimCarrier, handle->txChannel);
    }

    if (pulse->space != 0U)
    {
        IR_DelayUs(pulse->space);
    }
}
