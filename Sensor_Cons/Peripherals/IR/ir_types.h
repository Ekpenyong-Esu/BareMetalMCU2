/**
 * @file ir_types.h
 * @brief IR driver data types, protocol timings and error codes
 * @details Shared by every IR module. Contains no behaviour.
 */

#ifndef IR_TYPES_H
#define IR_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdbool.h>
#include <stdint.h>

/* Exported constants --------------------------------------------------------*/

/* IR Timing Constants (in microseconds) */
#define IR_CARRIER_FREQUENCY          38000U    /* 38 kHz carrier frequency */
#define IR_CARRIER_DUTY_CYCLE         33U       /* 33% duty cycle */
#define IR_TIMEOUT_MS                 1000U     /* 1 second timeout */

/** Counter rate the capture timer is prescaled to, one tick per microsecond */
#define IR_CAPTURE_TICK_FREQ          1000000U

/** Silence that marks the end of a frame, in microseconds */
#define IR_FRAME_GAP_US               10000U

/* NEC Protocol Constants */
#define IR_NEC_HEADER_MARK            9000U     /* 9ms header mark */
#define IR_NEC_HEADER_SPACE           4500U     /* 4.5ms header space */
#define IR_NEC_BIT_MARK               562U      /* 562us bit mark */
#define IR_NEC_BIT_0_SPACE            562U      /* 562us for bit 0 */
#define IR_NEC_BIT_1_SPACE            1687U     /* 1.687ms for bit 1 */
#define IR_NEC_STOP_BIT               562U      /* 562us stop bit */
#define IR_NEC_REPEAT_SPACE           2250U     /* 2.25ms repeat space */
#define IR_NEC_TOLERANCE              200U      /* 200us tolerance */
#define IR_NEC_DATA_BITS              32U       /* Address, ~address, command, ~command */
#define IR_NEC_HEADER_ENTRIES         2U        /* Buffer slots used by the header */

/* RC5 Protocol Constants */
#define IR_RC5_BIT_TIME               1778U     /* 1.778ms bit time */
#define IR_RC5_HALF_BIT               889U      /* Half bit time */
#define IR_RC5_TOLERANCE              300U      /* 300us tolerance */
#define IR_RC5_DATA_BITS              14U       /* 2 start bits + toggle + 5 addr + 6 cmd */
#define IR_RC5_START_BITS             0x03U     /* Both start bits are 1 */
#define IR_RC5_MAX_ADDRESS            31U       /* 5-bit address field */
#define IR_RC5_MAX_COMMAND            63U       /* 6-bit command field */

/* SIRC Protocol Constants */
#define IR_SIRC_HEADER_MARK           2400U     /* 2.4ms header */
#define IR_SIRC_BIT_MARK              600U      /* 600us bit mark */
#define IR_SIRC_BIT_0_SPACE           600U      /* 600us for bit 0 */
#define IR_SIRC_BIT_1_SPACE           1200U     /* 1.2ms for bit 1 */
#define IR_SIRC_TOLERANCE             200U      /* 200us tolerance */
#define IR_SIRC_DATA_BITS             12U       /* 7 command bits + 5 address bits */
#define IR_SIRC_HEADER_ENTRIES        1U        /* Buffer slots used by the header */
#define IR_SIRC_MAX_ADDRESS           31U       /* 5-bit address field */
#define IR_SIRC_MAX_COMMAND           127U      /* 7-bit command field */

/* IR Buffer Sizes */
#define IR_RX_BUFFER_SIZE             256U      /* Receive buffer size */
#define IR_TX_BUFFER_SIZE             128U      /* Transmit buffer size */
#define IR_MAX_PULSE_COUNT            100U      /* Maximum pulses per frame */

/* IR Protocols */
typedef enum {
    IR_PROTOCOL_NEC = 0,
    IR_PROTOCOL_RC5,
    IR_PROTOCOL_SIRC,
    IR_PROTOCOL_CUSTOM,
    IR_PROTOCOL_COUNT
} IR_Protocol_t;

/* IR States */
typedef enum {
    IR_STATE_IDLE = 0,
    IR_STATE_RECEIVING,
    IR_STATE_TRANSMITTING,
    IR_STATE_PROCESSING,
    IR_STATE_ERROR
} IR_State_t;

/* IR Events */
typedef enum {
    IR_EVENT_NONE = 0,
    IR_EVENT_FRAME_RECEIVED,
    IR_EVENT_FRAME_TRANSMITTED,
    IR_EVENT_REPEAT_RECEIVED,
    IR_EVENT_ERROR_TIMEOUT,
    IR_EVENT_ERROR_PROTOCOL,
    IR_EVENT_ERROR_OVERFLOW
} IR_Event_t;

/* Error Codes */
#define IR_ERROR_NONE                 0x00U
#define IR_ERROR_INIT                 0x01U
#define IR_ERROR_INVALID_PARAM        0x02U
#define IR_ERROR_TIMEOUT              0x03U
#define IR_ERROR_PROTOCOL             0x04U
#define IR_ERROR_BUFFER_OVERFLOW      0x05U
#define IR_ERROR_TX_BUSY              0x06U
#define IR_ERROR_RX_BUSY              0x07U
#define IR_ERROR_TIMER                0x08U

/* Exported types ------------------------------------------------------------*/

/**
 * @brief IR Pulse Structure
 */
typedef struct {
    uint16_t mark;                  /* Mark duration in microseconds */
    uint16_t space;                 /* Space duration in microseconds */
} IR_Pulse_t;

/**
 * @brief IR Frame Structure
 */
typedef struct {
    IR_Protocol_t protocol;         /* IR protocol type */
    uint32_t address;               /* Device address */
    uint32_t command;               /* Command code */
    uint32_t data;                  /* Raw data */
    uint8_t bits;                   /* Number of bits */
    bool repeat;                    /* Repeat flag */
    bool valid;                     /* Valid frame flag */
} IR_Frame_t;

/**
 * @brief IR Configuration Structure
 */
typedef struct {
    IR_Protocol_t protocol;         /* Default protocol */
    uint32_t carrierFreq;           /* Carrier frequency in Hz */
    uint8_t dutyCycle;              /* Duty cycle percentage */
    uint16_t tolerance;             /* Timing tolerance in microseconds */
    bool autoRepeat;                /* Auto repeat detection */
    bool invertSignal;              /* Invert output signal */
} IR_Config_t;

/**
 * @brief IR Handle Structure
 */
typedef struct {
    /* Hardware handles */
    TIM_HandleTypeDef *htimCarrier;     /* Timer for carrier generation */
    TIM_HandleTypeDef *htimCapture;     /* Timer for input capture */
    uint32_t txChannel;                 /* PWM channel for TX */
    uint32_t rxChannel;                 /* Input capture channel for RX */
    uint32_t captureTickFreq;           /* Capture counter rate in Hz */

    /* Configuration */
    IR_Config_t config;                 /* IR configuration */

    /* State variables */
    IR_State_t state;                   /* Current state */
    uint32_t errorCode;                 /* Last error code */
    bool initialized;                   /* Initialization flag */

    /* Receive data */
    uint32_t rxBuffer[IR_RX_BUFFER_SIZE];   /* Raw timing buffer */
    uint16_t rxIndex;                       /* Current buffer index */
    uint32_t lastCaptureTime;               /* Last capture timestamp */
    bool rxHasReference;                    /* lastCaptureTime holds a real edge */
    IR_Frame_t rxFrame;                     /* Received frame */

    /* Transmit data */
    IR_Pulse_t txBuffer[IR_TX_BUFFER_SIZE]; /* Transmit pulse buffer */
    uint16_t txIndex;                       /* Current transmit index */
    uint16_t txCount;                       /* Total pulses to transmit */

    /* Callback function */
    void (*eventCallback)(IR_Event_t event, IR_Frame_t *frame);
} IR_Handle_t;

#ifdef __cplusplus
}
#endif

#endif /* IR_TYPES_H */
