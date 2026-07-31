/**
 * @file heartbeat.h
 * @brief Generic table-driven heartbeat-pattern engine.
 * @details Plays any sequence of (state, duration) frames over an LED in an
 *          endless loop. The caller supplies the current time, so this module
 *          owns the player logic only — not the time source or the LED hardware.
 *
 *          Typical use: the classic "lub-dub ... rest" vital-signs LED pattern.
 *
 * Example
 * -------
 *   #include "led.h"
 *   #include "heartbeat.h"
 *
 *   static const HeartbeatFrame_t kPattern[] = {
 *       {LED_ON, 70}, {LED_OFF, 150}, {LED_ON, 70}, {LED_OFF, 900}
 *   };
 *
 *   Heartbeat_t hb;
 *   Heartbeat_Init(&hb, &myLed, kPattern, 4u);
 *   Heartbeat_Start(&hb, HAL_GetTick());
 *
 *   // in super-loop
 *   Heartbeat_Update(&hb, HAL_GetTick());
 */

#ifndef APP_HEARTBEAT_H
#define APP_HEARTBEAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "led.h"

/* Exported types ------------------------------------------------------------*/

/** A single phase of the pattern: hold @p state for @p durationMs. */
typedef struct {
    LedState_t state;
    uint32_t   durationMs;
} HeartbeatFrame_t;

typedef struct {
    LedHandle_t*             led;           /**< LED being driven */
    const HeartbeatFrame_t*  pattern;       /**< Pointer to frame table (static const) */
    uint32_t                 frameCount;    /**< Number of entries in the table */
    uint32_t                 currentFrame;  /**< Index of the active frame */
    uint32_t                 frameStartMs;  /**< Tick when the current frame began */
} Heartbeat_t;

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Bind an engine to an initialised LED and a frame table
 * @param  hb        Engine handle
 * @param  led       Pointer to an initialised LedHandle_t
 * @param  pattern   Static frame table (must live for the lifetime of the engine)
 * @param  frameCount Number of entries in @p pattern
 * @retval true on success
 */
bool Heartbeat_Init(Heartbeat_t* hb, LedHandle_t* led,
                    const HeartbeatFrame_t* pattern, uint32_t frameCount);

/**
 * @brief  Reset the engine to the first frame
 * @param  hb    Engine handle
 * @param  nowMs Current tick in milliseconds
 */
void Heartbeat_Start(Heartbeat_t* hb, uint32_t nowMs);

/**
 * @brief  Advance the engine. Call once per super-loop iteration.
 * @param  hb    Engine handle
 * @param  nowMs Current tick in milliseconds
 */
void Heartbeat_Update(Heartbeat_t* hb, uint32_t nowMs);

#ifdef __cplusplus
}
#endif

#endif /* APP_HEARTBEAT_H */
