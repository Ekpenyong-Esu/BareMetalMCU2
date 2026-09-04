/**
 * @file ir_types.h
 * @brief Shared types for the IR remote driver
 * @details IR remote sends invisible light pulses. The MCU reads the
 *          timing of those pulses to know which button was pressed.
 *          NEC, RC5 and SIRC are three common ways remotes blink the light.
 *          This file holds timings, settings and types for those.
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

/* Basic IR settings */
#define IR_CARRIER_FREQUENCY 38000U /* How fast the light blinks (38k times per sec) */
#define IR_CARRIER_DUTY_CYCLE 33U   /* Light is on 33% of the time */
#define IR_TIMEOUT_MS 1000U         /* Give up after 1 second with no signal */

/* Timer counts 1 tick per microsecond */
#define IR_CAPTURE_TICK_FREQ 1000000U

/* Long quiet time that means one message ended */
#define IR_FRAME_GAP_US 10000U

/* NEC remote - most common TV remote */
#define IR_NEC_HEADER_MARK 9000U  /* Long flash at start (9ms) */
#define IR_NEC_HEADER_SPACE 4500U /* Pause after start flash (4.5ms) */
#define IR_NEC_BIT_MARK 562U      /* Short flash for each bit */
#define IR_NEC_BIT_0_SPACE 562U   /* Short pause means bit 0 */
#define IR_NEC_BIT_1_SPACE 1687U  /* Long pause means bit 1 */
#define IR_NEC_STOP_BIT 562U      /* Final short flash */
#define IR_NEC_REPEAT_SPACE 2250U /* Pause for repeat press */
#define IR_NEC_TOLERANCE 200U     /* Allow 200us error in timing */
#define IR_NEC_DATA_BITS 32U      /* 32 bits: address + command */
#define IR_NEC_HEADER_ENTRIES 2U  /* Header uses 2 slots in buffer */
/* NEC frame layout, MSB first: address, ~address, command, ~command */
#define IR_NEC_ADDRESS_SHIFT 24U     /* Address byte position in the 32-bit frame */
#define IR_NEC_ADDRESS_INV_SHIFT 16U /* Inverted address byte position */
#define IR_NEC_COMMAND_SHIFT 8U      /* Command byte position */
#define IR_NEC_BYTE_MASK 0xFFU       /* One frame byte */

/* RC5 remote - Philips style */
#define IR_RC5_BIT_TIME 1778U   /* Time for one bit (1.778ms) */
#define IR_RC5_HALF_BIT 889U    /* Half bit time */
#define IR_RC5_TOLERANCE 300U   /* Allow 300us error */
#define IR_RC5_DATA_BITS 14U    /* 14 bits total */
#define IR_RC5_START_BITS 0x03U /* First two bits are always 1 */
#define IR_RC5_MAX_ADDRESS 31U  /* Biggest address value (5 bits) */
#define IR_RC5_MAX_COMMAND 63U  /* Biggest command value (6 bits) */
/* RC5 frame layout, MSB first: 2 start bits, toggle, 5 address, 6 command */
#define IR_RC5_START_SHIFT 12U  /* Start bits position in the 14-bit frame */
#define IR_RC5_ADDRESS_SHIFT 6U /* Address field position */

/* SIRC remote - Sony style */
#define IR_SIRC_HEADER_MARK 2400U /* Start flash (2.4ms) */
#define IR_SIRC_BIT_MARK 600U     /* Short flash for each bit */
#define IR_SIRC_BIT_0_SPACE 600U  /* Short pause means bit 0 */
#define IR_SIRC_BIT_1_SPACE 1200U /* Long pause means bit 1 */
#define IR_SIRC_TOLERANCE 200U    /* Allow 200us error */
#define IR_SIRC_DATA_BITS 12U     /* 12 bits: 7 command + 5 address */
#define IR_SIRC_HEADER_ENTRIES 1U /* Header uses 1 slot in buffer */
#define IR_SIRC_MAX_ADDRESS 31U   /* Biggest address value (5 bits) */
#define IR_SIRC_MAX_COMMAND 127U  /* Biggest command value (7 bits) */

/* Buffer sizes */
#define IR_RX_BUFFER_SIZE 256U  /* Space to store incoming pulses */
#define IR_TX_BUFFER_SIZE 128U  /* Space to store outgoing pulses */
#define IR_MAX_PULSE_COUNT 100U /* Most pulses in one message */

/** Which remote type is used */
typedef enum {
    IR_PROTOCOL_NEC = 0, /* NEC - most TV remotes */
    IR_PROTOCOL_RC5,     /* RC5 - Philips */
    IR_PROTOCOL_SIRC,    /* SIRC - Sony */
    IR_PROTOCOL_CUSTOM,  /* Your own pattern */
    IR_PROTOCOL_COUNT    /* How many types exist */
} IR_Protocol_t;

/** What the IR driver is doing now */
typedef enum {
    IR_STATE_IDLE = 0,     /* Doing nothing, waiting */
    IR_STATE_RECEIVING,    /* Getting a message */
    IR_STATE_TRANSMITTING, /* Sending a message */
    IR_STATE_PROCESSING,   /* Checking the message */
    IR_STATE_ERROR         /* Something went wrong */
} IR_State_t;

/** Events that can happen */
typedef enum {
    IR_EVENT_NONE = 0,          /* Nothing happened */
    IR_EVENT_FRAME_RECEIVED,    /* Got a full message */
    IR_EVENT_FRAME_TRANSMITTED, /* Sent a message */
    IR_EVENT_REPEAT_RECEIVED,   /* Got a repeat press (holding button) */
    IR_EVENT_ERROR_TIMEOUT,     /* Waited too long */
    IR_EVENT_ERROR_PROTOCOL,    /* Message did not match any remote type */
    IR_EVENT_ERROR_OVERFLOW     /* Too much data, buffer full */
} IR_Event_t;

/* Error codes - what went wrong */
#define IR_ERROR_NONE 0x00U            /* No error */
#define IR_ERROR_INIT 0x01U            /* Setup failed */
#define IR_ERROR_INVALID_PARAM 0x02U   /* Bad setting given */
#define IR_ERROR_TIMEOUT 0x03U         /* Took too long */
#define IR_ERROR_PROTOCOL 0x04U        /* Unknown remote pattern */
#define IR_ERROR_BUFFER_OVERFLOW 0x05U /* Buffer too full */
#define IR_ERROR_TX_BUSY 0x06U         /* Busy sending */
#define IR_ERROR_RX_BUSY 0x07U         /* Busy receiving */
#define IR_ERROR_TIMER 0x08U           /* Timer problem */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief One flash and pause pair
 * @details Mark is how long the light is on, space is how long it is off.
 */
typedef struct {
    uint16_t mark;  /* Light on time in microseconds */
    uint16_t space; /* Light off time in microseconds */
} IR_Pulse_t;

/**
 * @brief One full remote message
 * @details Holds who sent it, which button, and the raw bits.
 */
typedef struct {
    IR_Protocol_t protocol; /* Which remote type (NEC, RC5, etc) */
    uint32_t address;       /* Who the message is for */
    uint32_t command;       /* Which button was pressed */
    uint32_t data;          /* Raw bits as a number */
    uint8_t bits;           /* How many bits in the message */
    bool repeat;            /* True if button is held down */
    bool valid;             /* True if message looks good */
} IR_Frame_t;

/**
 * @brief Settings for the IR driver
 */
typedef struct {
    IR_Protocol_t protocol; /* Which remote type to use by default */
    uint32_t carrierFreq;   /* How fast the light blinks (Hz) */
    uint8_t dutyCycle;      /* How long light stays on (percent) */
    uint16_t tolerance;     /* How much timing error to allow (us) */
    bool autoRepeat;        /* Detect held button or not */
    bool invertSignal;      /* Flip the signal or not */
} IR_Config_t;

/**
 * @brief Main handle for the IR driver
 * @details Holds timers, settings, and buffers for send and receive.
 */
typedef struct {
    /* Timers and channels */
    TIM_HandleTypeDef *htimCarrier; /* Timer that makes the blink */
    TIM_HandleTypeDef *htimCapture; /* Timer that measures incoming blinks */
    uint32_t txChannel;             /* PWM channel for sending */
    uint32_t rxChannel;             /* Capture channel for receiving */
    uint32_t captureTickFreq;       /* Timer ticks per second */

    /* Settings */
    IR_Config_t config; /* Current settings */

    /* Status */
    IR_State_t state;   /* What the driver is doing now */
    uint32_t errorCode; /* Last error, 0 means no error */
    bool initialized;   /* True if setup is done */

    /* Data we received */
    uint32_t rxBuffer[IR_RX_BUFFER_SIZE]; /* Raw timings we captured */
    uint16_t rxIndex;                     /* Where we are in the buffer */
    uint32_t lastCaptureTime;             /* Time of last blink edge */
    bool rxHasReference;                  /* True if lastCaptureTime is valid */
    IR_Frame_t rxFrame;                   /* Last full message we got */

    /* Data to send */
    IR_Pulse_t txBuffer[IR_TX_BUFFER_SIZE]; /* Pulses to send */
    uint16_t txIndex;                       /* Which pulse we are on */
    uint16_t txCount;                       /* How many pulses to send */

    /* Called when something happens */
    void (*eventCallback)(IR_Event_t event, IR_Frame_t *frame);
} IR_Handle_t;

#ifdef __cplusplus
}
#endif

#endif /* IR_TYPES_H */
