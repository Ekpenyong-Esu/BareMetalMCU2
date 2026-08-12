/**
 * @file ir_decode.c
 * @brief Protocol decoding of a captured IR pulse train
 */

/* Includes ------------------------------------------------------------------*/
#include "ir_decode.h"
#include "ir_timing.h"

/* Private define ------------------------------------------------------------*/

/** Buffer entries a full NEC frame occupies: header pair plus one pair per bit */
#define IR_NEC_MIN_ENTRIES  (IR_NEC_HEADER_ENTRIES + (IR_NEC_DATA_BITS * 2U))

/** Buffer entries a full SIRC frame occupies */
#define IR_SIRC_MIN_ENTRIES (IR_SIRC_HEADER_ENTRIES + (IR_SIRC_DATA_BITS * 2U))

/** Buffer entries the simplified RC5 decoder reads */
#define IR_RC5_MIN_ENTRIES  (IR_RC5_DATA_BITS * 2U)

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef IR_DecodeNEC(IR_Handle_t *handle);
static HAL_StatusTypeDef IR_DecodeRC5(IR_Handle_t *handle);
static HAL_StatusTypeDef IR_DecodeSIRC(IR_Handle_t *handle);
static uint32_t IR_EntryMicroseconds(const IR_Handle_t *handle, uint16_t index);

/* Public functions ----------------------------------------------------------*/

HAL_StatusTypeDef IR_DecodeFrame(IR_Handle_t *handle)
{
    switch (handle->config.protocol)
    {
        case IR_PROTOCOL_NEC:
            return IR_DecodeNEC(handle);

        case IR_PROTOCOL_RC5:
            return IR_DecodeRC5(handle);

        case IR_PROTOCOL_SIRC:
            return IR_DecodeSIRC(handle);

        default:
            break;
    }

    /* Unknown protocol: try each decoder, every one of which stamps the frame
       with the protocol it recognised. */
    if (IR_DecodeNEC(handle) == HAL_OK)
    {
        return HAL_OK;
    }

    if (IR_DecodeRC5(handle) == HAL_OK)
    {
        return HAL_OK;
    }

    return IR_DecodeSIRC(handle);
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Duration of one captured interval in microseconds
 * @param handle: Pointer to IR handle structure
 * @param index: Receive buffer index
 * @return uint32_t: Duration in microseconds
 */
static uint32_t IR_EntryMicroseconds(const IR_Handle_t *handle, uint16_t index)
{
    return IR_TicksToMicroseconds(handle->rxBuffer[index], handle->captureTickFreq);
}

/**
 * @brief Decode NEC protocol
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef IR_DecodeNEC(IR_Handle_t *handle)
{
    const uint16_t tolerance = handle->config.tolerance;
    uint32_t data = 0;
    uint8_t address = 0U;
    uint8_t addressInv = 0U;
    uint8_t command = 0U;
    uint8_t commandInv = 0U;

    /* The bit loop reads two entries per bit after the header, so the whole
       frame has to be present before any of it is examined. */
    if (handle->rxIndex < IR_NEC_MIN_ENTRIES)
    {
        return HAL_ERROR;
    }

    uint32_t headerMark = IR_EntryMicroseconds(handle, 0);
    uint32_t headerSpace = IR_EntryMicroseconds(handle, 1);

    if (!IR_IsWithinTolerance(headerMark, IR_NEC_HEADER_MARK, tolerance) ||
        !IR_IsWithinTolerance(headerSpace, IR_NEC_HEADER_SPACE, tolerance))
    {
        return HAL_ERROR;
    }

    for (uint16_t i = 0; i < IR_NEC_DATA_BITS; i++)
    {
        const uint16_t markIndex = (uint16_t)(IR_NEC_HEADER_ENTRIES + (i * 2U));
        const uint32_t bitMark = IR_EntryMicroseconds(handle, markIndex);
        const uint32_t bitSpace = IR_EntryMicroseconds(handle, (uint16_t)(markIndex + 1U));

        if (!IR_IsWithinTolerance(bitMark, IR_NEC_BIT_MARK, tolerance))
        {
            return HAL_ERROR;
        }

        if (IR_IsWithinTolerance(bitSpace, IR_NEC_BIT_1_SPACE, tolerance))
        {
            data |= 1UL << (IR_NEC_DATA_BITS - 1U - i);   /* MSB first */
        }
        else if (!IR_IsWithinTolerance(bitSpace, IR_NEC_BIT_0_SPACE, tolerance))
        {
            return HAL_ERROR;
        }
    }

    address = (uint8_t)(data >> 24);
    addressInv = (uint8_t)(data >> 16);
    command = (uint8_t)(data >> 8);
    commandInv = (uint8_t)data;

    if ((address ^ addressInv) != 0xFFU || (command ^ commandInv) != 0xFFU)
    {
        return HAL_ERROR;
    }

    handle->rxFrame.protocol = IR_PROTOCOL_NEC;
    handle->rxFrame.address = address;
    handle->rxFrame.command = command;
    handle->rxFrame.data = data;
    handle->rxFrame.bits = IR_NEC_DATA_BITS;
    handle->rxFrame.repeat = false;

    return HAL_OK;
}

/**
 * @brief Decode RC5 protocol
 * @note  Simplified: a full-bit-time interval is read as a one. A complete
 *        implementation would track the Manchester phase across both halves.
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef IR_DecodeRC5(IR_Handle_t *handle)
{
    const uint16_t tolerance = handle->config.tolerance;
    uint16_t data = 0;
    uint8_t startBits;

    if (handle->rxIndex < IR_RC5_MIN_ENTRIES)
    {
        return HAL_ERROR;
    }

    for (uint16_t i = 0; i < IR_RC5_DATA_BITS; i++)
    {
        const uint32_t timing = IR_EntryMicroseconds(handle, (uint16_t)(i * 2U));

        if (IR_IsWithinTolerance(timing, IR_RC5_BIT_TIME, tolerance))
        {
            data |= (uint16_t)(1U << (IR_RC5_DATA_BITS - 1U - i));
        }
    }

    startBits = (uint8_t)((data >> 12) & 0x03U);
    if (startBits != IR_RC5_START_BITS)
    {
        return HAL_ERROR;
    }

    handle->rxFrame.protocol = IR_PROTOCOL_RC5;
    handle->rxFrame.address = (data >> 6) & IR_RC5_MAX_ADDRESS;
    handle->rxFrame.command = data & IR_RC5_MAX_COMMAND;
    handle->rxFrame.data = data;
    handle->rxFrame.bits = IR_RC5_DATA_BITS;
    handle->rxFrame.repeat = false;

    return HAL_OK;
}

/**
 * @brief Decode SIRC protocol
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef IR_DecodeSIRC(IR_Handle_t *handle)
{
    const uint16_t tolerance = handle->config.tolerance;
    uint16_t data = 0;

    if (handle->rxIndex < IR_SIRC_MIN_ENTRIES)
    {
        return HAL_ERROR;
    }

    if (!IR_IsWithinTolerance(IR_EntryMicroseconds(handle, 0), IR_SIRC_HEADER_MARK, tolerance))
    {
        return HAL_ERROR;
    }

    for (uint16_t i = 0; i < IR_SIRC_DATA_BITS; i++)
    {
        const uint16_t markIndex = (uint16_t)(IR_SIRC_HEADER_ENTRIES + (i * 2U));
        const uint32_t bitMark = IR_EntryMicroseconds(handle, markIndex);
        const uint32_t bitSpace = IR_EntryMicroseconds(handle, (uint16_t)(markIndex + 1U));

        if (!IR_IsWithinTolerance(bitMark, IR_SIRC_BIT_MARK, tolerance))
        {
            return HAL_ERROR;
        }

        if (IR_IsWithinTolerance(bitSpace, IR_SIRC_BIT_1_SPACE, tolerance))
        {
            data |= (uint16_t)(1U << i);    /* LSB first */
        }
        else if (!IR_IsWithinTolerance(bitSpace, IR_SIRC_BIT_0_SPACE, tolerance))
        {
            return HAL_ERROR;
        }
    }

    handle->rxFrame.protocol = IR_PROTOCOL_SIRC;
    handle->rxFrame.command = data & IR_SIRC_MAX_COMMAND;
    handle->rxFrame.address = (data >> 7) & IR_SIRC_MAX_ADDRESS;
    handle->rxFrame.data = data;
    handle->rxFrame.bits = IR_SIRC_DATA_BITS;
    handle->rxFrame.repeat = false;

    return HAL_OK;
}
