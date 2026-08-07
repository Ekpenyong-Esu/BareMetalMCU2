/**
 * @file ir_receive.c
 * @brief IR receive path: capture control, buffering and frame handover
 */

/* Includes ------------------------------------------------------------------*/
#include "ir_receive.h"
#include "ir_core.h"
#include "ir_decode.h"
#include "ir_timing.h"

/* Private function prototypes -----------------------------------------------*/
static void IR_ProcessReceivedData(IR_Handle_t *handle);

/* Public functions ----------------------------------------------------------*/

HAL_StatusTypeDef IR_StartReceive(IR_Handle_t *handle)
{
    HAL_StatusTypeDef status = HAL_OK;

    if (IR_ValidateHandle(handle) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (handle->state == IR_STATE_TRANSMITTING)
    {
        handle->errorCode = IR_ERROR_TX_BUSY;
        return HAL_ERROR;
    }

    IR_ResetReceiveBuffer(handle);

    status = HAL_TIM_IC_Start_IT(handle->htimCapture, handle->rxChannel);
    if (status != HAL_OK)
    {
        handle->errorCode = IR_ERROR_TIMER;
        return status;
    }

    handle->state = IR_STATE_RECEIVING;

    return HAL_OK;
}

HAL_StatusTypeDef IR_StopReceive(IR_Handle_t *handle)
{
    if (IR_ValidateHandle(handle) != HAL_OK)
    {
        return HAL_ERROR;
    }

    (void)HAL_TIM_IC_Stop_IT(handle->htimCapture, handle->rxChannel);

    handle->state = IR_STATE_IDLE;

    return HAL_OK;
}

HAL_StatusTypeDef IR_GetFrame(IR_Handle_t *handle, IR_Frame_t *frame)
{
    if (IR_ValidateHandle(handle) != HAL_OK || frame == NULL)
    {
        return HAL_ERROR;
    }

    if (!handle->rxFrame.valid)
    {
        return HAL_ERROR;
    }

    *frame = handle->rxFrame;
    handle->rxFrame.valid = false;

    return HAL_OK;
}

void IR_ResetReceiveBuffer(IR_Handle_t *handle)
{
    handle->rxIndex = 0;
    handle->lastCaptureTime = 0;
    handle->rxHasReference = false;
    handle->rxFrame.valid = false;
}

void IR_InputCaptureCallback(IR_Handle_t *handle, uint32_t captureValue)
{
    uint32_t elapsedTicks;

    if (handle == NULL || handle->state != IR_STATE_RECEIVING)
    {
        return;
    }

    /* The first edge of a frame has nothing to be measured against; storing it
       would put a raw counter value where the decoders expect a duration. */
    if (!handle->rxHasReference)
    {
        handle->lastCaptureTime = captureValue;
        handle->rxHasReference = true;
        return;
    }

    /* Unsigned wrap-around already yields the correct difference across a
       counter overflow, so no special case is needed. */
    elapsedTicks = captureValue - handle->lastCaptureTime;

    if (handle->rxIndex >= IR_RX_BUFFER_SIZE)
    {
        handle->errorCode = IR_ERROR_BUFFER_OVERFLOW;
        IR_NotifyEvent(handle, IR_EVENT_ERROR_OVERFLOW, NULL);
        IR_ResetReceiveBuffer(handle);
        return;
    }

    handle->rxBuffer[handle->rxIndex] = elapsedTicks;
    handle->rxIndex++;
    handle->lastCaptureTime = captureValue;

    if (IR_TicksToMicroseconds(elapsedTicks, handle->captureTickFreq) > IR_FRAME_GAP_US)
    {
        IR_ProcessReceivedData(handle);
    }
}

void IR_TimerOverflowCallback(IR_Handle_t *handle)
{
    if (handle == NULL || handle->state != IR_STATE_RECEIVING)
    {
        return;
    }

    if (handle->rxIndex > 0U)
    {
        IR_ProcessReceivedData(handle);
        return;
    }

    handle->errorCode = IR_ERROR_TIMEOUT;
    IR_NotifyEvent(handle, IR_EVENT_ERROR_TIMEOUT, NULL);
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Decode the captured pulse train and hand the result to the application
 * @param handle: Pointer to IR handle structure
 * @return void
 */
static void IR_ProcessReceivedData(IR_Handle_t *handle)
{
    handle->state = IR_STATE_PROCESSING;

    if (IR_DecodeFrame(handle) == HAL_OK)
    {
        handle->rxFrame.valid = true;
        IR_NotifyEvent(handle, IR_EVENT_FRAME_RECEIVED, &handle->rxFrame);
    }
    else
    {
        handle->errorCode = IR_ERROR_PROTOCOL;
        IR_NotifyEvent(handle, IR_EVENT_ERROR_PROTOCOL, NULL);
    }

    /* IR_ResetReceiveBuffer() clears the valid flag, so keep it and only drop
       the raw timings the next frame will overwrite. */
    handle->rxIndex = 0;
    handle->lastCaptureTime = 0;
    handle->rxHasReference = false;
    handle->state = IR_STATE_RECEIVING;
}
