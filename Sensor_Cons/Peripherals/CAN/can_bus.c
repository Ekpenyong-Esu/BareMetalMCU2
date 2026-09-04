/**
 ******************************************************************************
 * @file    can_bus.c
 * @brief   Backend-neutral CAN interface implementation
 *
 * This module hides the choice between on-chip bxCAN and external MCP2515
 * behind one CAN_Bus_Send/Receive API:
 * - Status translation: HAL / MCP2515 codes -> CAN_BusStatus
 * - CAN_Bus_Init/DeInit: dispatch to the configured backend
 * - CAN_Bus_Send/Receive/IsRxPending: frame conversion and backend dispatch
 *
 * Key Design Points:
 * - CAN_BusFrame is the common shape; converted to CAN_Frame or MCP2515_Frame_t
 * - MCP2515 has no blocking receive, so the wait loop lives here
 * - Timeout is honoured by bxCAN only; MCP2515 reports CAN_BUS_BUSY
 */

#include "can_bus.h"
#include <string.h>

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Translate an MCP2515 status to a bus status
 * @param status MCP2515 status
 * @retval CAN_BusStatus Corresponding bus status
 */
static CAN_BusStatus CAN_Bus_FromMcp(MCP2515_Status_t status) {
    switch (status) {
        case MCP2515_OK:
            return CAN_BUS_OK;
        case MCP2515_INVALID_PARAM:
            return CAN_BUS_INVALID_PARAM;
        case MCP2515_NOT_INITIALIZED:
            return CAN_BUS_NOT_INITIALIZED;
        case MCP2515_TIMEOUT:
            return CAN_BUS_TIMEOUT;
        case MCP2515_NO_MESSAGE:
            return CAN_BUS_NO_MESSAGE;
        case MCP2515_TX_BUSY:
            return CAN_BUS_BUSY;
        default:
            return CAN_BUS_ERROR;
    }
}

/**
 * @brief Translate a HAL status to a bus status
 * @param status HAL status
 * @retval CAN_BusStatus Corresponding bus status
 */
static CAN_BusStatus CAN_Bus_FromHal(HAL_StatusTypeDef status) {
    switch (status) {
        case HAL_OK:
            return CAN_BUS_OK;
        case HAL_TIMEOUT:
            return CAN_BUS_TIMEOUT;
        case HAL_BUSY:
            return CAN_BUS_BUSY;
        default:
            return CAN_BUS_ERROR;
    }
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Bring up whichever controller config names
 * @param bus Bus handle to initialize (must be zeroed or uninitialized)
 * @param config Backend selection and controller config
 * @retval CAN_BusStatus CAN_BUS_OK on success
 */

CAN_BusStatus CAN_Bus_Init(CAN_Bus *bus, const CAN_BusConfig *config) {
    if (bus == NULL || config == NULL) {
        return CAN_BUS_INVALID_PARAM;
    }

    memset(bus, 0, sizeof(*bus));
    bus->backend = config->backend;

    if (config->backend == CAN_BUS_BACKEND_BXCAN) {
        HAL_StatusTypeDef status = CAN_Init(&bus->dev.bxcan, &config->cfg.bxcan);

        if (status != HAL_OK) {
            return CAN_Bus_FromHal(status);
        }
    }
    else if (config->backend == CAN_BUS_BACKEND_MCP2515) {
        MCP2515_Status_t status = MCP2515_Init(&bus->dev.mcp2515, &config->cfg.mcp2515);

        if (status != MCP2515_OK) {
            return CAN_Bus_FromMcp(status);
        }
    }
    else {
        return CAN_BUS_INVALID_PARAM;
    }

    bus->initialized = true;

    return CAN_BUS_OK;
}

/**
 * @brief Tear down the active backend
 * @param bus Bus handle (must be initialized)
 * @retval CAN_BusStatus CAN_BUS_OK on success
 */
CAN_BusStatus CAN_Bus_DeInit(CAN_Bus *bus) {
    CAN_BusStatus status = CAN_BUS_OK;

    if (bus == NULL) {
        return CAN_BUS_INVALID_PARAM;
    }
    if (!bus->initialized) {
        return CAN_BUS_NOT_INITIALIZED;
    }

    status = (bus->backend == CAN_BUS_BACKEND_BXCAN)
                 ? CAN_Bus_FromHal(CAN_DeInit(&bus->dev.bxcan))
                 : CAN_Bus_FromMcp(MCP2515_DeInit(&bus->dev.mcp2515));

    if (status == CAN_BUS_OK) {
        bus->initialized = false;
    }

    return status;
}

/**
 * @brief Queue a frame for transmission
 * @param bus Bus handle (must be initialized)
 * @param frame Frame to send (length 0..8)
 * @param timeout Max wait in ms (bxCAN only; MCP2515 is non-blocking)
 * @retval CAN_BusStatus CAN_BUS_OK, CAN_BUS_BUSY or error
 */
CAN_BusStatus CAN_Bus_Send(CAN_Bus *bus, const CAN_BusFrame *frame, uint32_t timeout) {
    if (bus == NULL || frame == NULL || frame->length > 8U) {
        return CAN_BUS_INVALID_PARAM;
    }
    if (!bus->initialized) {
        return CAN_BUS_NOT_INITIALIZED;
    }

    if (bus->backend == CAN_BUS_BACKEND_BXCAN) {
        CAN_Frame out = {0};

        out.id = frame->id;
        out.frame_type = frame->extended ? CAN_FRAME_EXTENDED : CAN_FRAME_STANDARD;
        out.remote_request = frame->remote;
        out.data_length = frame->length;
        memcpy(out.data, frame->data, frame->length);

        return CAN_Bus_FromHal(CAN_Transmit(&bus->dev.bxcan, &out, timeout));
    }

    MCP2515_Frame_t out = {0};

    out.id = frame->id;
    out.extended = frame->extended;
    out.remote = frame->remote;
    out.length = frame->length;
    memcpy(out.data, frame->data, frame->length);

    return CAN_Bus_FromMcp(MCP2515_Transmit(&bus->dev.mcp2515, &out));
}

/**
 * @brief Check whether any frame is waiting
 * @param bus Bus handle
 * @retval bool true when a frame is available
 */
bool CAN_Bus_IsRxPending(CAN_Bus *bus) {
    if (bus == NULL || !bus->initialized) {
        return false;
    }

    if (bus->backend == CAN_BUS_BACKEND_BXCAN) {
        return CAN_IsReceivePending(&bus->dev.bxcan, 0) || CAN_IsReceivePending(&bus->dev.bxcan, 1);
    }

    return MCP2515_IsMessagePending(&bus->dev.mcp2515);
}

/**
 * @brief Receive the next pending frame
 * @param bus Bus handle (must be initialized)
 * @param frame Output frame
 * @param timeout Max wait in ms
 * @retval CAN_BusStatus CAN_BUS_OK on success, CAN_BUS_NO_MESSAGE on timeout
 */
CAN_BusStatus CAN_Bus_Receive(CAN_Bus *bus, CAN_BusFrame *frame, uint32_t timeout) {
    if (bus == NULL || frame == NULL) {
        return CAN_BUS_INVALID_PARAM;
    }
    if (!bus->initialized) {
        return CAN_BUS_NOT_INITIALIZED;
    }

    memset(frame, 0, sizeof(*frame));

    if (bus->backend == CAN_BUS_BACKEND_BXCAN) {
        CAN_Frame rxFrame = {0};
        HAL_StatusTypeDef status = CAN_Receive(&bus->dev.bxcan, &rxFrame, timeout);

        if (status != HAL_OK) {
            /* An empty FIFO is a normal poll result, not a bus fault. */
            return (status == HAL_TIMEOUT || status == HAL_ERROR) ? CAN_BUS_NO_MESSAGE
                                                                  : CAN_Bus_FromHal(status);
        }

        frame->id = rxFrame.id;
        frame->extended = (rxFrame.frame_type == CAN_FRAME_EXTENDED);
        frame->remote = rxFrame.remote_request;
        frame->length = (rxFrame.data_length > 8U) ? 8U : rxFrame.data_length;
        memcpy(frame->data, rxFrame.data, frame->length);

        return CAN_BUS_OK;
    }

    /* The MCP2515 driver has no blocking read, so the wait lives here. */
    uint32_t start = HAL_GetTick();
    while (!MCP2515_IsMessagePending(&bus->dev.mcp2515)) {
        if ((HAL_GetTick() - start) >= timeout) {
            return CAN_BUS_NO_MESSAGE;
        }
    }

    MCP2515_Frame_t rxFrame = {0};
    MCP2515_Status_t status = MCP2515_Receive(&bus->dev.mcp2515, &rxFrame);

    if (status != MCP2515_OK) {
        return CAN_Bus_FromMcp(status);
    }

    frame->id = rxFrame.id;
    frame->extended = rxFrame.extended;
    frame->remote = rxFrame.remote;
    frame->length = (rxFrame.length > 8U) ? 8U : rxFrame.length;
    memcpy(frame->data, rxFrame.data, frame->length);

    return CAN_BUS_OK;
}
