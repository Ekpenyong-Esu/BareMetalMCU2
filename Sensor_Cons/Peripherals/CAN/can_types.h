/**
  ******************************************************************************
  * @file    can_types.h
  * @brief   Shared vocabulary for the CAN driver
  ******************************************************************************
  */

#ifndef CAN_TYPES_H
#define CAN_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include "stm32f4xx_hal_can.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define CAN_MAX_DATA_LENGTH         8U
#define CAN_MAX_FILTER_BANKS         14U
#define CAN_MAX_MAILBOXES            3U

/* Baud rate presets (bps) */
#define CAN_BAUD_1000KBPS            1000000U
#define CAN_BAUD_500KBPS             500000U
#define CAN_BAUD_250KBPS             250000U
#define CAN_BAUD_125KBPS             125000U
#define CAN_BAUD_100KBPS             100000U
#define CAN_BAUD_50KBPS              50000U
#define CAN_BAUD_20KBPS              20000U
#define CAN_BAUD_10KBPS              10000U

typedef enum {
    CAN_FRAME_STANDARD = 0U,
    CAN_FRAME_EXTENDED = 1U
} CAN_FrameType;

typedef enum {
    CAN_ERROR_NONE = 0,
    CAN_ERROR_STUFF,
    CAN_ERROR_FORM,
    CAN_ERROR_ACK,
    CAN_ERROR_BIT_RECESSIVE,
    CAN_ERROR_BIT_DOMINANT,
    CAN_ERROR_CRC,
    CAN_ERROR_BUS_OFF,
    CAN_ERROR_BUS_PASSIVE,
    CAN_ERROR_BUS_WARNING
} CAN_ErrorType;

typedef uint32_t CAN_OperatingMode;

#define CAN_OP_MODE_NORMAL           CAN_MODE_NORMAL
#define CAN_OP_MODE_LOOPBACK         CAN_MODE_LOOPBACK
#define CAN_OP_MODE_SILENT           CAN_MODE_SILENT
#define CAN_OP_MODE_SILENT_LOOPBACK  CAN_MODE_SILENT_LOOPBACK

typedef uint32_t CAN_FilterMode;
typedef uint32_t CAN_FilterScale;

#define CAN_FILTER_MODE_ID_MASK      CAN_FILTERMODE_IDMASK
#define CAN_FILTER_MODE_ID_LIST     CAN_FILTERMODE_IDLIST
#define CAN_FILTER_SCALE_16BIT      CAN_FILTERSCALE_16BIT
#define CAN_FILTER_SCALE_32BIT      CAN_FILTERSCALE_32BIT

typedef struct {
    uint32_t id;
    CAN_FrameType frame_type;
    uint8_t data_length;
    uint8_t data[CAN_MAX_DATA_LENGTH];
    bool remote_request;
    uint32_t timestamp;
} CAN_Frame;

typedef struct {
    CAN_OperatingMode mode;
    uint32_t baud_rate;
    bool auto_retransmission;
    bool auto_bus_off_recovery;
    bool time_triggered_comm;
    uint32_t prescaler;
    uint32_t sync_jump_width;
    uint32_t time_segment_1;
    uint32_t time_segment_2;
} CAN_Config;

typedef struct {
    uint8_t filter_bank;
    CAN_FilterMode mode;
    CAN_FilterScale scale;
    CAN_FrameType frame_type;  /**< Decides where the id is placed in the bank */
    uint32_t id;
    uint32_t mask;
    uint8_t fifo;              /**< Receive FIFO the match is routed to, 0 or 1 */
    bool enable;
} CAN_FilterConfig;

typedef struct {
    bool initialized;
    uint32_t tx_mailbox_free;
    uint32_t rx_messages_pending;
    CAN_ErrorType last_error;
    uint32_t error_count;
    uint32_t tx_count;
    uint32_t rx_count;
} CAN_Status;

/* Callback function types */
typedef void (*CAN_TxCallback)(uint8_t mailbox);
typedef void (*CAN_RxCallback)(const CAN_Frame *frame);
typedef void (*CAN_ErrorCallbackFn)(CAN_ErrorType error);

#ifdef __cplusplus
}
#endif

#endif /* CAN_TYPES_H */
