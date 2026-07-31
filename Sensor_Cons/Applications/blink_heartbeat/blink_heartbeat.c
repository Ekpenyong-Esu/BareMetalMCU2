/**
 * @file    blink_heartbeat.c
 * @brief   Application 1 - Blink & Heartbeat implementation.
 *
 * Orchestrates two independent LED behaviours in one cooperative, tick-based loop:
 *   - Activity LED (green PG13): alternates between a fixed-rate blink and a
 *     breathing fade produced by the software-PWM module.
 *   - Heartbeat LED (red PG14):   classic "lub-dub ... rest" double-pulse,
 *     driven by the reusable Heartbeat_t engine.
 *
 * Each behaviour is a separate module — this file wires them together.
 * All timing is derived from HAL_GetTick() / SYS_GetMicros(); no delays.
 */

#include "blink_heartbeat.h"

#include "board.h"
#include "heartbeat.h"
#include "led.h"
#include "led_blink.h"
#include "led_pwm.h"
#include "sys.h"

/* -------------------------------------------------------------------------- */
/* Configuration - tune the demo here.                                        */
/* -------------------------------------------------------------------------- */

#define ACTIVITY_BLINK_PERIOD_MS   500u   /* Full on+off period of the activity blink  */
#define ACTIVITY_PHASE_MS          5000u  /* How long before switching blink↔breathe  */
#define BREATHE_PERIOD_MS          3000u  /* Fade-in + fade-out duration                */

/* -------------------------------------------------------------------------- */
/* Heartbeat pattern data.                                                    */
/* -------------------------------------------------------------------------- */

static const HeartbeatFrame_t kHeartbeatPattern[] = {
    { LED_ON,   70u },   /* lub  */
    { LED_OFF, 150u },   /* gap between beats */
    { LED_ON,   70u },   /* dub  */
    { LED_OFF, 900u },   /* rest  */
};

#define HEARTBEAT_FRAME_COUNT (sizeof(kHeartbeatPattern) / sizeof(kHeartbeatPattern[0]))

/* -------------------------------------------------------------------------- */
/* Module state.                                                              */
/* -------------------------------------------------------------------------- */

typedef enum {
    ACTIVITY_PHASE_BLINK = 0,
    ACTIVITY_PHASE_BREATHE
} ActivityPhase_t;

static LedHandle_t s_activityLed;    /* Green PG13: blink / breathe. */
static LedHandle_t s_heartbeatLed;   /* Red   PG14: heartbeat.       */

static LedBlink_t s_activityBlink;
static LedPwm_t   s_activityPwm;
static Heartbeat_t s_heartbeat;

static ActivityPhase_t s_activityPhase;
static uint32_t s_activityPhaseStart;  /* ms tick when the phase began */

/* -------------------------------------------------------------------------- */
/* Activity LED: blink ⇄ breathe alternation (private).                       */
/* -------------------------------------------------------------------------- */

static void Activity_EnterPhase(ActivityPhase_t phase, uint32_t nowMs)
{
    s_activityPhase = phase;
    s_activityPhaseStart = nowMs;

    if (phase == ACTIVITY_PHASE_BLINK) {
        LedPwm_Stop(&s_activityPwm);
        LedBlink_Start(&s_activityBlink, ACTIVITY_BLINK_PERIOD_MS, nowMs);
    } else {
        LedBlink_Stop(&s_activityBlink);
        LedPwm_SetBrightness(&s_activityPwm, 0u);
        LedPwm_Start(&s_activityPwm, SYS_GetMicros());
    }
}

static void Activity_Task(uint32_t nowMs)
{
    uint32_t elapsedMs = nowMs - s_activityPhaseStart;

    if (elapsedMs >= ACTIVITY_PHASE_MS) {
        ActivityPhase_t next = (s_activityPhase == ACTIVITY_PHASE_BLINK)
                             ? ACTIVITY_PHASE_BREATHE
                             : ACTIVITY_PHASE_BLINK;
        Activity_EnterPhase(next, nowMs);
        return;
    }

    if (s_activityPhase == ACTIVITY_PHASE_BLINK) {
        LedBlink_Update(&s_activityBlink, nowMs);
    } else {
        uint8_t brightness = LedPwm_Waveform_Smooth(elapsedMs, BREATHE_PERIOD_MS);
        LedPwm_SetBrightness(&s_activityPwm, brightness);
        LedPwm_Update(&s_activityPwm, SYS_GetMicros());
    }
}

/* -------------------------------------------------------------------------- */
/* Public API.                                                                */
/* -------------------------------------------------------------------------- */

bool App_BlinkHeartbeat_Init(void)
{
    const LedConfig_t activityConfig = {
        .port = BOARD_LED_GREEN_PORT,
        .pin  = BOARD_LED_GREEN_PIN,
        .activeLow = BOARD_LED_ACTIVE_LOW
    };
    const LedConfig_t heartbeatConfig = {
        .port = BOARD_LED_RED_PORT,
        .pin  = BOARD_LED_RED_PIN,
        .activeLow = BOARD_LED_ACTIVE_LOW
    };

    if (!Led_InitCustom(&s_activityLed, &activityConfig) ||
        !Led_InitCustom(&s_heartbeatLed, &heartbeatConfig)) {
        return false;
    }

    if (!LedBlink_Init(&s_activityBlink, &s_activityLed) ||
        !LedPwm_Init(&s_activityPwm, &s_activityLed, LED_PWM_DEFAULT_PERIOD_US) ||
        !Heartbeat_Init(&s_heartbeat, &s_heartbeatLed, kHeartbeatPattern, HEARTBEAT_FRAME_COUNT)) {
        return false;
    }

    uint32_t nowMs = HAL_GetTick();

    Activity_EnterPhase(ACTIVITY_PHASE_BLINK, nowMs);
    Heartbeat_Start(&s_heartbeat, nowMs);
    return true;
}

void App_BlinkHeartbeat_Task(void)
{
    uint32_t nowMs = HAL_GetTick();

    Activity_Task(nowMs);
    Heartbeat_Update(&s_heartbeat, nowMs);
}

void App_BlinkHeartbeat_Run(void)
{
    if (!App_BlinkHeartbeat_Init()) {
        Error_Handler();
    }

    for (;;) {
        App_BlinkHeartbeat_Task();
    }
}
