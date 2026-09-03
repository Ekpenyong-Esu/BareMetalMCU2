/**
 * @file tone_player_app.c
 * @brief Application 6, "Tone Player": beeps and melodies on a buzzer
 *
 * This is the only file that knows anything about the board. Each layer below
 * it depends only on the next one down, and none of them depend back up:
 *
 *   tone_player_app  which pin, which timer, which tunes to play
 *   tone_console     saying over the serial port what is playing
 *   tone_player      pitch and tempo turned into buzzer activity
 *   melody           the order notes come in, and the tempo
 *   note             what one note's pitch is, in hertz
 *   BUZZER / TIM     the drivers
 *
 * Progress is reported on USART1 at 115200 8N1 (PA9 TX, PA10 RX).
 *
 * Wiring, depending on the mode passed to TonePlayerApp_Run():
 *
 *   passive  buzzer between PE5 (header P1-14) and ground, driven by TIM9_CH1
 *   active   buzzer between PE6 (header P1-11) and ground, driven as an output
 */

#include "tone_player_app.h"

#include "gpio.h"
#include "melody.h"
#include "tone_console.h"
#include "tone_player.h"

/* PE5 and PE6 are two of the few pins this board leaves unused, they are
   brought out side by side on P1, and PE5 is TIM9_CH1. */
#define TONE_PLAYER_APP_PASSIVE_PORT      GPIOE
#define TONE_PLAYER_APP_PASSIVE_PIN       GPIO_PIN_5
#define TONE_PLAYER_APP_PASSIVE_ALTERNATE GPIO_AF3_TIM9
#define TONE_PLAYER_APP_PASSIVE_TIMER     TIM9
#define TONE_PLAYER_APP_PASSIVE_CHANNEL   TIM_CHANNEL_1

#define TONE_PLAYER_APP_ACTIVE_PORT GPIOE
#define TONE_PLAYER_APP_ACTIVE_PIN  GPIO_PIN_6

/* The console prints these verbatim: which pin is wired up is board knowledge,
   and board knowledge lives in this file. */
#define TONE_PLAYER_APP_PASSIVE_DESCRIPTION "passive buzzer on PE5, driven by TIM9_CH1"
#define TONE_PLAYER_APP_ACTIVE_DESCRIPTION  "active buzzer on PE6, rhythm only at its own fixed pitch"

#define TONE_PLAYER_APP_READY_BEEP_HZ 1000U
#define TONE_PLAYER_APP_READY_BEEP_MS 120U

/** Silence between tunes, so it is obvious where one ends. */
#define TONE_PLAYER_APP_PAUSE_MS 1500U

#define TONE_PLAYER_APP_PLAYLIST_LENGTH (sizeof(s_playlist) / sizeof(s_playlist[0]))

/* Static so they outlive TonePlayerApp_Run's stack frame: the timer keeps
   running off this handle for as long as the buzzer sounds. */
static TIM_HandleTypeDef s_buzzerTimer;
static Buzzer_t s_buzzer;

static const Melody_t *const s_playlist[] = {
    &MELODY_C_MAJOR_SCALE,
    &MELODY_TWINKLE,
    &MELODY_ODE_TO_JOY,
};

/** @brief Hand the buzzer pin over to the timer, then bind the buzzer to it */
static HAL_StatusTypeDef TonePlayerApp_InitPassiveBuzzer(void)
{
    GPIO_InitTypeDef gpioInit = {0};

    gpioInit.Pin       = TONE_PLAYER_APP_PASSIVE_PIN;
    gpioInit.Mode      = GPIO_MODE_AF_PP;
    gpioInit.Pull      = GPIO_NOPULL;
    gpioInit.Speed     = GPIO_SPEED_FREQ_LOW; /* audio is slow; a faster edge only adds noise */
    gpioInit.Alternate = TONE_PLAYER_APP_PASSIVE_ALTERNATE;

    if (GPIO_Driver_Pin_Init(TONE_PLAYER_APP_PASSIVE_PORT, &gpioInit) != HAL_OK) {
        return HAL_ERROR;
    }

    return TonePlayer_InitPassive(&s_buzzer, &s_buzzerTimer,
                                  TONE_PLAYER_APP_PASSIVE_TIMER,
                                  TONE_PLAYER_APP_PASSIVE_CHANNEL);
}

/** @brief Bring up whichever buzzer the caller has actually wired up */
static HAL_StatusTypeDef TonePlayerApp_InitBuzzer(BuzzerMode_t mode)
{
    if (mode == BUZZER_MODE_ACTIVE_GPIO) {
        /* Buzzer_InitActive configures its own pin, so there is nothing to
           prepare beforehand the way the passive path needs. */
        return Buzzer_InitActive(&s_buzzer,
                                 TONE_PLAYER_APP_ACTIVE_PORT,
                                 TONE_PLAYER_APP_ACTIVE_PIN);
    }

    return TonePlayerApp_InitPassiveBuzzer();
}

/** @brief Work through the playlist once, announcing each tune first */
static void TonePlayerApp_PlayPlaylist(void)
{
    for (uint32_t i = 0U; i < TONE_PLAYER_APP_PLAYLIST_LENGTH; i++) {
        ToneConsole_ReportMelody(s_playlist[i]);
        TonePlayer_PlayMelody(&s_buzzer, s_playlist[i]);
        HAL_Delay(TONE_PLAYER_APP_PAUSE_MS);
    }
}

void TonePlayerApp_Run(BuzzerMode_t mode)
{
    /* Opened first so that a buzzer that will not start can say so. */
    ToneConsole_Init();

    if (TonePlayerApp_InitBuzzer(mode) != HAL_OK) {
        ToneConsole_ReportError("the buzzer output could not be started");
        return; /* the caller decides what to do */
    }

    ToneConsole_ReportReady(mode == BUZZER_MODE_ACTIVE_GPIO
                                ? TONE_PLAYER_APP_ACTIVE_DESCRIPTION
                                : TONE_PLAYER_APP_PASSIVE_DESCRIPTION);

    /* One beep before the first tune, so a silent board is a wiring problem
       rather than a tune that has not started yet. */
    TonePlayer_Beep(&s_buzzer,
                    TONE_PLAYER_APP_READY_BEEP_HZ,
                    TONE_PLAYER_APP_READY_BEEP_MS);

    for (;;) {
        TonePlayerApp_PlayPlaylist();
    }
}
