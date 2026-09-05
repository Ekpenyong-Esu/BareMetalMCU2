/**
 * @file servo_sweep_app.c
 * @brief Application 7, "Servo Sweep": drive a hobby servo through its range
 *
 * This is the only file that knows anything about the board. It sits on top
 * of two modules that do not know about each other:
 *
 *   servo_sweep_app   which pin, which timer, which UART, which sweep to run
 *     |            \
 *   servo_sweep     servo_console
 *   (angles and     (saying over the serial
 *    timing turned   port what is sweeping)
 *    into motion)         |
 *     |                 UART driver
 *   SERVO / TIM drivers
 *
 * Progress is reported on USART1 at 115200 8N1 (PA9 TX, PA10 RX).
 *
 * Wiring:
 *
 *   servo A signal (orange/yellow) to PB4, driven by TIM3_CH1
 *   servo B signal (orange/yellow) to PC8, driven by TIM3_CH3
 *   servo VCC (red) to a separate 5-6 V supply, never to the MCU pin
 *   servo GND (brown/black) to supply GND and board GND together
 */

#include "servo_sweep_app.h"

#include "servo_console.h"
#include "servo_sweep.h"

/* PB4 and PC8 are free on this board (no FMC/LTDC/I2C claim in project.ioc) and
   carry TIM3_CH1 and TIM3_CH3, which the SERVO driver maps via GPIO_AF2_TIM3.
   TIM3's other two channels only reach pins the LCD and SDRAM already own. */
#define SERVO_SWEEP_APP_TIMER TIM3
#define SERVO_SWEEP_APP_A_PORT GPIOB
#define SERVO_SWEEP_APP_A_PIN GPIO_PIN_4
#define SERVO_SWEEP_APP_A_CHANNEL TIM_CHANNEL_1
#define SERVO_SWEEP_APP_B_PORT GPIOC
#define SERVO_SWEEP_APP_B_PIN GPIO_PIN_8
#define SERVO_SWEEP_APP_B_CHANNEL TIM_CHANNEL_3

/* The ST-LINK virtual COM port on this board. */
#define SERVO_SWEEP_APP_UART USART1

/* The console prints these verbatim: which pin is wired up is board knowledge,
   and board knowledge lives in this file. */
#define SERVO_SWEEP_APP_DESCRIPTION "servo A on PB4 (TIM3_CH1), servo B on PC8 (TIM3_CH3)"
#define SERVO_SWEEP_APP_A_NAME "servo A on PB4"
#define SERVO_SWEEP_APP_B_NAME "servo B on PC8"

/** Pause between full sweeps, so the turnaround point is visible on UART. */
#define SERVO_SWEEP_APP_PAUSE_MS 1000U

/* Static so they outlive ServoSweepApp_Run's stack frame: the timer keeps
   running off these handles for as long as the servos move. Both servos share
   one timer, so they share the 50 Hz frame TIM3 is programmed for. */
static TIM_HandleTypeDef s_servoTimer;
static SERVO_Handle_t s_servoA;
static SERVO_Handle_t s_servoB;

/** @brief Turn a driver status into words the console can print */
static const char *ServoSweepApp_StatusText(SERVO_StatusTypeDef status) {
    switch (status) {
        case SERVO_OK:
            return "ok";
        case SERVO_ERROR:
            return "driver error";
        case SERVO_BUSY:
            return "servo busy";
        case SERVO_TIMEOUT:
            return "servo timed out";
        case SERVO_INVALID_PARAM:
            return "invalid parameter";
        case SERVO_NOT_INITIALIZED:
            return "servo not initialised";
        case SERVO_OUT_OF_RANGE:
            return "angle out of range";
        default:
            return "unknown status";
    }
}

/** @brief Sweep one servo once, naming it first so the angles can be placed */
static SERVO_StatusTypeDef ServoSweepApp_SweepOne(SERVO_Handle_t *hservo, const char *name,
                                                  const ServoSweep_Config_t *sweep) {
    ServoConsole_ReportTurn(name);
    return ServoSweep_RunOnce(hservo, sweep, ServoConsole_ReportAngle);
}

void ServoSweepApp_Run(void) {
    /* Opened first so that a servo that will not start can say so. */
    ServoConsole_Init(SERVO_SWEEP_APP_UART);

    /* SERVO_Init reads the peripheral off the handle; it never picks one itself. */
    s_servoTimer.Instance = SERVO_SWEEP_APP_TIMER;

    SERVO_StatusTypeDef status = SERVO_Init(&s_servoA, &s_servoTimer, SERVO_SWEEP_APP_A_CHANNEL,
                                            SERVO_SWEEP_APP_A_PORT, SERVO_SWEEP_APP_A_PIN);
    if (status != SERVO_OK) {
        ServoConsole_ReportError(ServoSweepApp_StatusText(status));
        return; /* the caller decides what to do */
    }

    /* Re-programs TIM3 with the same 50 Hz frame, so channel 1 keeps running. */
    status = SERVO_Init(&s_servoB, &s_servoTimer, SERVO_SWEEP_APP_B_CHANNEL,
                        SERVO_SWEEP_APP_B_PORT, SERVO_SWEEP_APP_B_PIN);
    if (status != SERVO_OK) {
        ServoConsole_ReportError(ServoSweepApp_StatusText(status));
        return;
    }

    ServoConsole_ReportReady(SERVO_SWEEP_APP_DESCRIPTION);

    const ServoSweep_Config_t sweep = ServoSweep_GetDefaultConfig(&s_servoA);

    for (;;) {
        status = ServoSweepApp_SweepOne(&s_servoA, SERVO_SWEEP_APP_A_NAME, &sweep);
        if (status != SERVO_OK) {
            ServoConsole_ReportError(ServoSweepApp_StatusText(status));
            return;
        }

        status = ServoSweepApp_SweepOne(&s_servoB, SERVO_SWEEP_APP_B_NAME, &sweep);
        if (status != SERVO_OK) {
            ServoConsole_ReportError(ServoSweepApp_StatusText(status));
            return;
        }

        HAL_Delay(SERVO_SWEEP_APP_PAUSE_MS);
    }
}
