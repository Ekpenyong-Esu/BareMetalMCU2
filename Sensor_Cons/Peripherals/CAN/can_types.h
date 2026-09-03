/**
  ******************************************************************************
  * @file    can_types.h
  * @brief   Shared vocabulary for the CAN driver
  * @details Configuration, handle and frame constants. Every other translation
  *          unit in the module depends on this header and nothing else.
  *
  * CAN Driver Architecture:
  * - can_types.h:     Types, constants, handle definition (this file)
  * - can_core.h/c:    Lifecycle (init/deinit), mode switching, handle registry
  * - can_baudrate.h/c: Bit-timing solver (PCLK1-aware, any clock tree)
  * - can_filter.h/c:  bxCAN filter-bank packing (32-bit and 16-bit scale)
  * - can_transfer.h/c: Frame transmit/receive, mailbox and FIFO helpers
  * - can_events.h/c:  Callback registration and HAL ISR dispatch
  * - can_bus.h/c:     Backend-neutral facade (bxCAN or MCP2515 over SPI)
  * - can.h:           Aggregator that pulls the whole driver in one include
  *
  * Multi-CAN Support:
  * - CAN1 and CAN2 can be used simultaneously via caller-owned handles
  * - Each handle is self-contained; the registry in can_core.c maps
  *   CAN instance -> handle for HAL MspInit and ISR callbacks
  * - Filter banks are shared between CAN1/CAN2; the split is config-driven
  *
  * Backend Choice:
  * - On-chip bxCAN when CAN-capable pins are free
  * - External MCP2515 over SPI when they are not (this board: LCD/SDRAM/USB
  *   claim every CAN-capable pin, so MCP2515 is the only working option)
  * - can_bus.h hides the choice behind one CAN_Bus_Send/Receive API
  *
  * Frame Format:
  * - Classic CAN 2.0B: 0..8 data bytes, 11-bit standard or 29-bit extended ID
  * - Remote frames (RTR) carry no data; DLC still indicates requested length
  ******************************************************************************
  */

#ifndef CAN_TYPES_H
#define CAN_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_hal_can.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Exported constants --------------------------------------------------------*/

#define CAN_MAX_DATA_LENGTH         8U    /*!< Classic CAN payload limit (bytes) */
#define CAN_MAX_FILTER_BANKS        14U   /*!< Filter banks on STM32F429 (single CAN); dual-CAN parts share 28 */
#define CAN_MAX_MAILBOXES           3U    /*!< bxCAN transmit mailboxes */

/* Baud rate presets (bps) — common CiA choices, all derived by the solver */
#define CAN_BAUD_1000KBPS           1000000U  /*!< 1 Mbps — short bus, high speed */
#define CAN_BAUD_500KBPS            500000U   /*!< 500 kbps — typical automotive */
#define CAN_BAUD_250KBPS            250000U   /*!< 250 kbps — longer bus */
#define CAN_BAUD_125KBPS            125000U   /*!< 125 kbps — robust, long bus */
#define CAN_BAUD_100KBPS            100000U   /*!< 100 kbps — low-speed fault-tolerant */
#define CAN_BAUD_50KBPS             50000U    /*!< 50 kbps — very long bus */
#define CAN_BAUD_20KBPS             20000U    /*!< 20 kbps — legacy / diagnostics */
#define CAN_BAUD_10KBPS             10000U    /*!< 10 kbps — minimum practical rate */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief CAN identifier format
 */
typedef enum {
    CAN_FRAME_STANDARD = 0U,  /*!< 11-bit identifier (0x000..0x7FF) */
    CAN_FRAME_EXTENDED = 1U   /*!< 29-bit identifier (0x00000000..0x1FFFFFFF) */
} CAN_FrameType;

/**
 * @brief Bus error classification
 * @note  Ordered by severity in can_core.c so the first HAL flag match wins;
 *        bus-off must not be hidden by a stuff error reported at the same time.
 */
typedef enum {
    CAN_ERROR_NONE = 0,       /*!< No error */
    CAN_ERROR_STUFF,          /*!< Stuff error — bit-stuffing rule violated */
    CAN_ERROR_FORM,           /*!< Form error — fixed-form field malformed */
    CAN_ERROR_ACK,            /*!< ACK error — no receiver acknowledged */
    CAN_ERROR_BIT_RECESSIVE,  /*!< Bit error: transmitted dominant, read recessive */
    CAN_ERROR_BIT_DOMINANT,   /*!< Bit error: transmitted recessive, read dominant */
    CAN_ERROR_CRC,            /*!< CRC mismatch */
    CAN_ERROR_BUS_OFF,        /*!< Transmit error counter > 255, controller off-bus */
    CAN_ERROR_BUS_PASSIVE,    /*!< Error counter > 127, passive error state */
    CAN_ERROR_BUS_WARNING     /*!< Error counter > 96, warning threshold */
} CAN_ErrorType;

/** @brief Operating mode — aliases HAL CAN_MODE_* for board-agnostic config */
typedef uint32_t CAN_OperatingMode;

#define CAN_OP_MODE_NORMAL           CAN_MODE_NORMAL          /*!< Normal bus participation */
#define CAN_OP_MODE_LOOPBACK         CAN_MODE_LOOPBACK        /*!< Internal loopback, no transceiver needed */
#define CAN_OP_MODE_SILENT           CAN_MODE_SILENT          /*!< Listen-only, never drives the bus */
#define CAN_OP_MODE_SILENT_LOOPBACK  CAN_MODE_SILENT_LOOPBACK /*!< Silent + loopback for self-test */

/** @brief Filter match rule — aliases HAL CAN_FILTERMODE_* */
typedef uint32_t CAN_FilterMode;
/** @brief Filter bank width — aliases HAL CAN_FILTERSCALE_* */
typedef uint32_t CAN_FilterScale;

#define CAN_FILTER_MODE_ID_MASK      CAN_FILTERMODE_IDMASK  /*!< Mask mode: (id & mask) == (filter & mask) */
#define CAN_FILTER_MODE_ID_LIST      CAN_FILTERMODE_IDLIST  /*!< List mode: id equals one of two stored IDs */
#define CAN_FILTER_SCALE_16BIT       CAN_FILTERSCALE_16BIT  /*!< Two 16-bit filters per bank */
#define CAN_FILTER_SCALE_32BIT       CAN_FILTERSCALE_32BIT  /*!< One 32-bit filter per bank */

/**
 * @brief One CAN frame (Classic CAN 2.0B)
 */
typedef struct {
    uint32_t id;                          /*!< 11-bit (standard) or 29-bit (extended) identifier */
    CAN_FrameType frame_type;             /*!< Standard vs extended */
    uint8_t data_length;                  /*!< DLC: 0..8 */
    uint8_t data[CAN_MAX_DATA_LENGTH];    /*!< Payload; only data_length bytes are valid */
    bool remote_request;                  /*!< True for RTR (remote) frames — no data, requests DLC bytes */
    uint32_t timestamp;                   /*!< Free-running timer captured at reception (bxCAN) */
} CAN_Frame;

/**
 * @brief Pins the transceiver is wired to
 * @note  There is no default: on this board every CAN-capable pin is already
 *        taken by the LCD, the SDRAM or USB, so the caller has to say which
 *        of those it is giving up. The alternate function is always AF9 on
 *        STM32F4 but varies on other families, so it is config-driven.
 */
typedef struct {
    GPIO_TypeDef *tx_port;    /*!< TX port (e.g. GPIOB) */
    uint16_t      tx_pin;     /*!< TX pin mask (e.g. GPIO_PIN_9) */
    GPIO_TypeDef *rx_port;    /*!< RX port */
    uint16_t      rx_pin;     /*!< RX pin mask */
    uint8_t       af;         /*!< GPIO alternate function; 0 selects AF9 (bxCAN on STM32F4) */
} CAN_PinConfig;

/**
 * @brief CAN bus configuration
 *
 * All parameters map to HAL CAN_InitTypeDef fields or board-specific wiring.
 * instance: CAN1/CAN2; NULL defaults to CAN1
 * pins:     TX/RX port/pin/AF — no default, caller must supply
 * mode:     CAN_OP_MODE_NORMAL / LOOPBACK / SILENT / SILENT_LOOPBACK
 * baud_rate: preset (CAN_BAUD_*); 0 means use raw prescaler/sync_jump_width/time_segment_*
 * filter_bank_count: total banks; 0 defaults to 14 (STM32F429 single-CAN)
 * slave_start_bank:  first bank owned by CAN2; 0 defaults to filter_bank_count
 */
typedef struct {
    CAN_TypeDef *instance;        /*!< CAN1 or CAN2; NULL selects CAN1 */
    CAN_PinConfig pins;           /*!< Transceiver wiring — no default on this board */
    CAN_OperatingMode mode;       /*!< Operating mode */
    uint32_t baud_rate;           /*!< Preset baud rate (bps); 0 = use raw timing fields */
    bool auto_retransmission;     /*!< Retransmit automatically on arbitration loss / error */
    bool auto_bus_off_recovery;   /*!< Recover from bus-off without software intervention */
    bool time_triggered_comm;     /*!< Time-triggered communication mode */
    uint32_t prescaler;           /*!< Raw prescaler when baud_rate == 0 (1..1024) */
    uint32_t sync_jump_width;     /*!< Raw SJW when baud_rate == 0 */
    uint32_t time_segment_1;      /*!< Raw BS1 when baud_rate == 0 */
    uint32_t time_segment_2;      /*!< Raw BS2 when baud_rate == 0 */
    uint8_t filter_bank_count;    /*!< Total filter banks; 0 selects 14 */
    uint8_t slave_start_bank;     /*!< First bank owned by CAN2; 0 selects filter_bank_count */
} CAN_Config;

/**
 * @brief Filter bank configuration
 */
typedef struct {
    uint8_t filter_bank;      /*!< Bank index (0 .. filter_bank_count-1) */
    CAN_FilterMode mode;      /*!< Mask vs list */
    CAN_FilterScale scale;    /*!< 16-bit (two filters) vs 32-bit (one filter) */
    CAN_FrameType frame_type; /*!< Decides where the id is placed in the bank */
    uint32_t id;              /*!< Identifier to match */
    uint32_t mask;            /*!< Mask (mask mode) or second ID (list mode) */
    uint8_t fifo;             /*!< Receive FIFO the match is routed to, 0 or 1 */
    bool enable;              /*!< Activate this bank */
} CAN_FilterConfig;

/**
 * @brief Live bus status snapshot
 */
typedef struct {
    bool initialized;               /*!< True after successful CAN_Init */
    uint32_t tx_mailbox_free;       /*!< Free transmit mailboxes (0..3) */
    uint32_t rx_messages_pending;   /*!< Frames waiting across both FIFOs */
    CAN_ErrorType last_error;       /*!< Most recent error (sticky until cleared) */
    uint32_t error_count;           /*!< Total errors recorded */
    uint32_t tx_count;              /*!< Frames transmitted */
    uint32_t rx_count;              /*!< Frames received */
} CAN_Status;

/* Callback types — carry the handle so one function can serve several buses */
typedef struct CAN_Handle CAN_Handle_t;

typedef void (*CAN_TxCallback)(CAN_Handle_t *hcan, uint8_t mailbox);        /*!< Transmit complete on mailbox */
typedef void (*CAN_RxCallback)(CAN_Handle_t *hcan, const CAN_Frame *frame); /*!< Frame received */
typedef void (*CAN_ErrorCallbackFn)(CAN_Handle_t *hcan, CAN_ErrorType error); /*!< Bus error detected */

/**
 * @brief One CAN bus — caller-owned handle
 *
 * Self-contained: no file-scope state backs it, so several handles on
 * different CAN instances may exist at once. The registry in can_core.c
 * maps CAN instance -> handle for HAL MspInit and ISR callbacks.
 */
struct CAN_Handle {
    CAN_HandleTypeDef   hal;              /*!< HAL handle (Instance + Init fields) */
    CAN_Config          config;           /*!< Copy of the config passed to CAN_Init */
    CAN_Status          status;           /*!< Counters and last error */
    CAN_TxCallback      tx_callback;      /*!< Transmit-complete callback (or NULL) */
    CAN_RxCallback      rx_callback;      /*!< Receive callback (or NULL) */
    CAN_ErrorCallbackFn error_callback;   /*!< Error callback (or NULL) */
};

#ifdef __cplusplus
}
#endif

#endif /* CAN_TYPES_H */
