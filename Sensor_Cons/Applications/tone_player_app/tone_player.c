/**
 * @file tone_player.c
 * @brief Plays notes and melodies through a passive buzzer
 */

#include "tone_player.h"

#include "tim_clock.h"
#include "tim_pwm.h"

#include <stddef.h>

/* A passive buzzer is a square-wave device, so half on and half off is both
   the loudest and the cleanest tone it can produce. */
#define TONE_PLAYER_DUTY_PERCENT 50U

/* Silence at the end of every step, so two identical notes in a row are heard
   as two notes rather than one long one. */
#define TONE_PLAYER_NOTE_GAP_MS 20U

/* The value does not matter: Buzzer_Tone rewrites the prescaler and the reload
   for every note. This only gets the channel into PWM mode with a compare of
   zero, which leaves the output quiet until the first note. */
#define TONE_PLAYER_INITIAL_PERIOD 999U

HAL_StatusTypeDef TonePlayer_InitPassive(Buzzer_t *buzzer,
                                         TIM_HandleTypeDef *htim,
                                         TIM_TypeDef *instance,
                                         uint32_t channel)
{
    if (buzzer == NULL || htim == NULL) {
        return HAL_ERROR;
    }

    if (!TIM_Clock_HasOutputChannels(instance) || !TIM_Clock_Enable(instance)) {
        return HAL_ERROR;
    }

    if (TIM_PWM_Init(htim, instance, 0U, TONE_PLAYER_INITIAL_PERIOD) != HAL_OK) {
        return HAL_ERROR;
    }

    /* Without this the channel keeps its reset output-compare mode and never
       produces a waveform, whatever the compare register is later set to. */
    if (TIM_PWM_ConfigChannel(htim, channel, 0U, TIM_OCPOLARITY_HIGH) != HAL_OK) {
        return HAL_ERROR;
    }

    return Buzzer_InitPassive(buzzer, htim, channel, TIM_Clock_GetHz(instance));
}

void TonePlayer_Beep(Buzzer_t *buzzer, uint32_t frequencyHz, uint32_t durationMs)
{
    if (buzzer == NULL) {
        return;
    }

    if (buzzer->mode == BUZZER_MODE_ACTIVE_GPIO) {
        /* Its oscillator is built in, so the pitch is fixed and the pin only
           decides whether it is heard. */
        Buzzer_On(buzzer);
    } else if (Buzzer_Tone(buzzer, frequencyHz, TONE_PLAYER_DUTY_PERCENT) != HAL_OK) {
        /* Out of the timer's reach; stay silent instead of playing a pitch
           that is not the one asked for. */
        Buzzer_Off(buzzer);
    }

    HAL_Delay(durationMs);
    Buzzer_Off(buzzer);
}

void TonePlayer_PlayNote(Buzzer_t *buzzer,
                         NoteName_t name,
                         uint8_t octave,
                         uint32_t durationMs)
{
    const uint32_t frequencyHz = Note_FrequencyHz(name, octave);

    if (frequencyHz == 0U) {
        Buzzer_Off(buzzer); /* a rest is a note you wait through */
        HAL_Delay(durationMs);
        return;
    }

    TonePlayer_Beep(buzzer, frequencyHz, durationMs);
}

void TonePlayer_PlayMelody(Buzzer_t *buzzer, const Melody_t *melody)
{
    if (buzzer == NULL || melody == NULL || melody->steps == NULL) {
        return;
    }

    for (uint16_t i = 0U; i < melody->stepCount; i++) {
        const MelodyStep_t *step = &melody->steps[i];
        uint32_t durationMs = Melody_StepDurationMs(melody, step->length);

        /* Take the gap out of the note rather than adding it to the beat, so
           the tempo stays right however many notes the tune has. */
        if (durationMs > TONE_PLAYER_NOTE_GAP_MS) {
            durationMs -= TONE_PLAYER_NOTE_GAP_MS;
        }

        TonePlayer_PlayNote(buzzer, step->name, step->octave, durationMs);

        Buzzer_Off(buzzer);
        HAL_Delay(TONE_PLAYER_NOTE_GAP_MS);
    }
}
