/**
 ******************************************************************************
 * @file    can_bus.h
 * @brief   Backend-neutral CAN interface
 * @details Lets an application talk CAN without committing to how the board
 *          provides it. Boards whose CAN-capable pins are free use the
 *          on-chip bxCAN controller; boards where those pins are taken —
 *          this one, where the LCD, the SDRAM and USB claim every option —
 *          use an MCP2515 over SPI instead. Only the CAN_BusConfig changes
 *          between the two.
 *
 * CAN Bus Responsibilities:
 * - CAN_Bus_Init/DeInit: Bring up or tear down whichever backend is configured
 * - CAN_Bus_Send/Receive: Frame exchange through a common CAN_BusFrame shape
 * - CAN_Bus_IsRxPending: Non-blocking poll for waiting frames
 * - Status translation: HAL / MCP2515 codes -> CAN_BusStatus
 *
 * Backend Choice:
 * - CAN_BUS_BACKEND_BXCAN: on-chip bxCAN (needs free CAN-capable pins)
 * - CAN_BUS_BACKEND_MCP2515: external MCP2515 over SPI (this board's option)
 * - The SPI bus must already be running before MCP2515 init
 ******************************************************************************
 */

#ifndef CAN_BUS_H
#define CAN_BUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "can_core.h"
#include "can_transfer.h"
#include "mcp2515_core.h"
#include "mcp2515_transfer.h"

/**
 * @brief Backend that actually drives the bus
 */
typedef enum {
    CAN_BUS_BACKEND_BXCAN = 0, /*!< On-chip bxCAN controller */
    CAN_BUS_BACKEND_MCP2515    /*!< External MCP2515 on the SPI bus */
} CAN_BusBackend;

/**
 * @brief Backend-neutral status
 */
typedef enum {
    CAN_BUS_OK = 0,          /*!< Success */
    CAN_BUS_ERROR,           /*!< Generic failure */
    CAN_BUS_INVALID_PARAM,   /*!< Null or out-of-range argument */
    CAN_BUS_NOT_INITIALIZED, /*!< Bus not yet initialized */
    CAN_BUS_TIMEOUT,         /*!< Operation timed out */
    CAN_BUS_NO_MESSAGE,      /*!< Nothing waiting; not a fault */
    CAN_BUS_BUSY             /*!< Previous frame still queued */
} CAN_BusStatus;

/**
 * @brief Frame shape common to both backends (Classic CAN 2.0B, 0..8 bytes)
 */
typedef struct {
    uint32_t id;     /*!< 11-bit (standard) or 29-bit (extended) identifier */
    bool extended;   /*!< True for 29-bit extended frames */
    bool remote;     /*!< True for RTR (remote) frames */
    uint8_t length;  /*!< DLC: 0..8 */
    uint8_t data[8]; /*!< Payload; only length bytes are valid */
} CAN_BusFrame;

/**
 * @brief Backend-neutral bus configuration
 */
typedef struct {
    CAN_BusBackend backend; /*!< Which controller to use */
    union {
        CAN_Config bxcan;         /*!< bxCAN config when backend == BXCAN */
        MCP2515_Config_t mcp2515; /*!< MCP2515 config when backend == MCP2515 */
    } cfg;
} CAN_BusConfig;

/**
 * @brief Backend-neutral bus handle — caller-owned
 */
typedef struct {
    CAN_BusBackend backend; /*!< Active backend (from CAN_Bus_Init) */
    bool initialized;       /*!< True after successful CAN_Bus_Init */
    union {
        CAN_Handle_t bxcan;       /*!< bxCAN handle when backend == BXCAN */
        MCP2515_Handle_t mcp2515; /*!< MCP2515 handle when backend == MCP2515 */
    } dev;
} CAN_Bus;

/**
 * @brief   Bring up whichever controller @p config names
 * @param   bus Bus handle to initialize (must be zeroed or uninitialized)
 * @param   config Backend selection and controller config
 * @retval  CAN_BusStatus CAN_BUS_OK on success
 * @note    For the MCP2515 backend the SPI bus itself must already be running.
 */
CAN_BusStatus CAN_Bus_Init(CAN_Bus *bus, const CAN_BusConfig *config);

/**
 * @brief   Tear down the active backend
 * @param   bus Bus handle (must be initialized)
 * @retval  CAN_BusStatus CAN_BUS_OK on success
 */
CAN_BusStatus CAN_Bus_DeInit(CAN_Bus *bus);

/**
 * @brief   Queue a frame for transmission
 * @param   bus Bus handle (must be initialized)
 * @param   frame Frame to send (length 0..8)
 * @param   timeout Max wait in ms (honoured by bxCAN only; MCP2515 is
 *          non-blocking and reports CAN_BUS_BUSY instead of waiting)
 * @retval  CAN_BusStatus CAN_BUS_OK, CAN_BUS_BUSY or error
 */
CAN_BusStatus CAN_Bus_Send(CAN_Bus *bus, const CAN_BusFrame *frame, uint32_t timeout);

/**
 * @brief   Receive the next pending frame
 * @param   bus Bus handle (must be initialized)
 * @param   frame Output frame
 * @param   timeout Max wait in ms
 * @retval  CAN_BusStatus CAN_BUS_OK on success, CAN_BUS_NO_MESSAGE when
 *          nothing arrived within timeout
 */
CAN_BusStatus CAN_Bus_Receive(CAN_Bus *bus, CAN_BusFrame *frame, uint32_t timeout);

/**
 * @brief   Check whether any frame is waiting
 * @param   bus Bus handle
 * @retval  bool true when a frame is available
 */
bool CAN_Bus_IsRxPending(CAN_Bus *bus);

#ifdef __cplusplus
}
#endif

#endif /* CAN_BUS_H */
