/**
 * @file led_pattern.h
 * @brief Table-driven LED pattern player.
 * @details Owns one responsibility: tracking which frame of a (state, duration)
 *          table is current. It does not touch an LED, so the same pattern can
 *          be rendered as plain on/off, as software PWM, or as timer PWM.
 *
 *          The caller supplies the current time, so this module owns the
 *          sequencing logic only - not the time source or the output stage.
 *
 * Example
 * -------
 *   static const LedPatternFrame_t kPattern[] = {
 *       {LED_ON, 70}, {LED_OFF, 150}, {LED_ON, 70}, {LED_OFF, 900}
 *   };
 *
 *   LedPattern_t pattern;
 *   LedPattern_Init(&pattern, kPattern, 4u);
 *   LedPattern_Start(&pattern, HAL_GetTick());
 *
 *   // in super-loop
 *   LedPattern_Update(&pattern, HAL_GetTick());
 *   Led_SetState(&led, LedPattern_GetState(&pattern));
 */

#ifndef LED_PATTERN_H
#define LED_PATTERN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "led.h"

/* Exported types ------------------------------------------------------------*/

/** A single phase of the pattern: hold @p state for @p durationMs. */
typedef struct {
    LedState_t state;
    uint32_t durationMs;
} LedPatternFrame_t;

typedef struct {
    const LedPatternFrame_t *frames; /**< Frame table (must stay allocated) */
    uint32_t frameCount;             /**< Number of entries in the table */
    uint32_t currentFrame;           /**< Index of the active frame */
    uint32_t frameStartMs;           /**< Tick when the current frame began */
} LedPattern_t;

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Bind a player to a frame table
 * @param  pattern    Player handle
 * @param  frames     Static frame table (must live for the lifetime of the player)
 * @param  frameCount Number of entries in @p frames
 * @retval true on success
 */
bool LedPattern_Init(LedPattern_t *pattern, const LedPatternFrame_t *frames, uint32_t frameCount);

/**
 * @brief  Reset the player to the first frame
 * @param  pattern Player handle
 * @param  nowMs   Current tick in milliseconds
 */
void LedPattern_Start(LedPattern_t *pattern, uint32_t nowMs);

/**
 * @brief  Advance to the next frame when the current one has elapsed
 * @param  pattern Player handle
 * @param  nowMs   Current tick in milliseconds
 */
void LedPattern_Update(LedPattern_t *pattern, uint32_t nowMs);

/**
 * @brief  Get the on/off state of the current frame
 * @param  pattern Player handle
 * @retval LED_ON or LED_OFF
 */
LedState_t LedPattern_GetState(const LedPattern_t *pattern);

/**
 * @brief  Get how long the current frame has been running
 * @details Lets a caller shape brightness across a frame without this module
 *          having to pick a waveform for it.
 * @param  pattern Player handle
 * @param  nowMs   Current tick in milliseconds
 * @retval Milliseconds since the current frame began
 */
uint32_t LedPattern_GetElapsedMs(const LedPattern_t *pattern, uint32_t nowMs);

#ifdef __cplusplus
}
#endif

#endif /* LED_PATTERN_H */
