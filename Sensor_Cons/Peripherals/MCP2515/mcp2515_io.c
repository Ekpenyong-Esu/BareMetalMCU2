/**
  ******************************************************************************
  * @file    mcp2515_io.c
  * @brief   Register-level access to the MCP2515 over the shared SPI bus
  ******************************************************************************
  */

#include "mcp2515_io.h"
#include "mcp2515_regs.h"
#include "spi_transfer.h"

/* Private functions ---------------------------------------------------------*/

static void MCP2515_CsLow(MCP2515_Handle_t *hmcp)
{
    HAL_GPIO_WritePin(hmcp->cs_port, hmcp->cs_pin, GPIO_PIN_RESET);
}

static void MCP2515_CsHigh(MCP2515_Handle_t *hmcp)
{
    HAL_GPIO_WritePin(hmcp->cs_port, hmcp->cs_pin, GPIO_PIN_SET);
}

/** @brief Common entry guard: a handle without a registered device cannot talk */
static MCP2515_Status_t MCP2515_CheckReady(MCP2515_Handle_t *hmcp)
{
    if (hmcp == NULL) {
        return MCP2515_INVALID_PARAM;
    }
    if (!SPI_DeviceIsReady(&hmcp->device)) {
        return MCP2515_NOT_INITIALIZED;
    }
    return MCP2515_OK;
}

/**
 * @brief Frame one exchange: assert CS, send the header, then move the payload.
 * @note  CS is released on every path so a failed transfer cannot leave the
 *        device selected and block the display and gyro sharing this bus.
 */
static MCP2515_Status_t MCP2515_Exchange(MCP2515_Handle_t *hmcp,
                                         const uint8_t *header, uint8_t headerLength,
                                         uint8_t *rxData, const uint8_t *txData,
                                         uint8_t length)
{
    SPI_StatusTypeDef status;

    MCP2515_CsLow(hmcp);

    status = SPI_Transmit(&hmcp->device, (uint8_t *)header, headerLength, MCP2515_SPI_TIMEOUT);
    if (status == SPI_OK && length > 0U) {
        if (rxData != NULL) {
            status = SPI_Receive(&hmcp->device, rxData, length, MCP2515_SPI_TIMEOUT);
        } else {
            status = SPI_Transmit(&hmcp->device, (uint8_t *)txData, length, MCP2515_SPI_TIMEOUT);
        }
    }

    MCP2515_CsHigh(hmcp);

    return (status == SPI_OK) ? MCP2515_OK : MCP2515_ERROR;
}

/* Exported functions --------------------------------------------------------*/

MCP2515_Status_t MCP2515_ReadRegisters(MCP2515_Handle_t *hmcp, uint8_t reg,
                                       uint8_t *data, uint8_t length)
{
    const uint8_t header[2] = { MCP2515_CMD_READ, reg };

    MCP2515_Status_t status = MCP2515_CheckReady(hmcp);
    if (status != MCP2515_OK) {
        return status;
    }
    if (data == NULL || length == 0U) {
        return MCP2515_INVALID_PARAM;
    }

    return MCP2515_Exchange(hmcp, header, sizeof(header), data, NULL, length);
}

MCP2515_Status_t MCP2515_ReadRegister(MCP2515_Handle_t *hmcp, uint8_t reg, uint8_t *value)
{
    return MCP2515_ReadRegisters(hmcp, reg, value, 1U);
}

MCP2515_Status_t MCP2515_WriteRegisters(MCP2515_Handle_t *hmcp, uint8_t reg,
                                        const uint8_t *data, uint8_t length)
{
    const uint8_t header[2] = { MCP2515_CMD_WRITE, reg };

    MCP2515_Status_t status = MCP2515_CheckReady(hmcp);
    if (status != MCP2515_OK) {
        return status;
    }
    if (data == NULL || length == 0U) {
        return MCP2515_INVALID_PARAM;
    }

    return MCP2515_Exchange(hmcp, header, sizeof(header), NULL, data, length);
}

MCP2515_Status_t MCP2515_WriteRegister(MCP2515_Handle_t *hmcp, uint8_t reg, uint8_t value)
{
    return MCP2515_WriteRegisters(hmcp, reg, &value, 1U);
}

MCP2515_Status_t MCP2515_ModifyRegister(MCP2515_Handle_t *hmcp, uint8_t reg,
                                        uint8_t mask, uint8_t value)
{
    const uint8_t frame[4] = { MCP2515_CMD_BIT_MODIFY, reg, mask, value };

    MCP2515_Status_t status = MCP2515_CheckReady(hmcp);
    if (status != MCP2515_OK) {
        return status;
    }

    return MCP2515_Exchange(hmcp, frame, sizeof(frame), NULL, NULL, 0U);
}

MCP2515_Status_t MCP2515_Command(MCP2515_Handle_t *hmcp, uint8_t command)
{
    MCP2515_Status_t status = MCP2515_CheckReady(hmcp);
    if (status != MCP2515_OK) {
        return status;
    }

    return MCP2515_Exchange(hmcp, &command, 1U, NULL, NULL, 0U);
}

MCP2515_Status_t MCP2515_ResetDevice(MCP2515_Handle_t *hmcp)
{
    MCP2515_Status_t status = MCP2515_Command(hmcp, MCP2515_CMD_RESET);
    if (status != MCP2515_OK) {
        return status;
    }

    /* The oscillator has to restart before the device answers again. */
    HAL_Delay(10U);

    return MCP2515_OK;
}
