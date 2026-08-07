/**
 * @file ts_events.c
 * @brief Touch interrupt setup and deferred servicing
 */

#include "ts_events.h"
#include "ts_core.h"
#include "ts_io.h"
#include "ts_gesture.h"
#include "ts_stmpe811.h"
#include "gpio.h"
#include "log.h"
#include "app_low_power.h"

static volatile bool s_irqPending = false;  /*!< Set by the EXTI ISR, consumed by TS_ServiceIRQ() */

TS_StatusTypeDef TS_EnableInterrupt(TS_HandleTypeDef *hts, bool enable)
{
    if (hts == NULL) {
        return TS_INVALID_PARAM;
    }

    if (enable) {
        log_debug("TS: Enabling touchscreen interrupt");
        /* FIFO threshold as well as touch detect, so movement keeps reporting. */
        TS_WriteRegister(hts, STMPE811_REG_INT_EN, STMPE811_INT_EN_TOUCH_DET | STMPE811_INT_EN_FIFO_TH);
        TS_WriteRegister(hts, STMPE811_REG_INT_CTRL,
                         STMPE811_INT_CTRL_POL_LOW | STMPE811_INT_CTRL_EDGE | STMPE811_INT_CTRL_ENABLE);
    } else {
        TS_WriteRegister(hts, STMPE811_REG_INT_EN, 0x00);
        TS_WriteRegister(hts, STMPE811_REG_INT_CTRL, STMPE811_INT_CTRL_DISABLE);
    }

    return TS_OK;
}

TS_StatusTypeDef TS_ITConfig(TS_HandleTypeDef *hts)
{
    if (hts == NULL) {
        return TS_INVALID_PARAM;
    }

    GPIO_InitTypeDef gpioInit = {0};
    gpioInit.Pin = TS_INT_PIN;
    gpioInit.Mode = GPIO_MODE_IT_FALLING;
    gpioInit.Pull = GPIO_PULLUP;
    gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_Driver_Pin_Init(TS_INT_GPIO_PORT, &gpioInit);

    HAL_NVIC_SetPriority(TS_INT_EXTI_IRQn, TS_INT_NVIC_PRIORITY, 0x00);
    HAL_NVIC_EnableIRQ(TS_INT_EXTI_IRQn);

    log_debug("TS_ITConfig Initialized");
    return TS_OK;
}

bool TS_IrqPending(void)
{
    return s_irqPending;
}

void TS_IRQHandler(TS_HandleTypeDef *hts)
{
    if (hts == NULL) {
        return;
    }

    uint8_t intStatus = 0;
    TS_ReadRegister(hts, STMPE811_REG_INT_STA, &intStatus);

    if (intStatus & STMPE811_INT_EN_TOUCH_DET) {
        if (hts->TouchCallback != NULL) {
            hts->TouchCallback();
        }

        TS_GestureTypeDef gesture = TS_AnalyzeGesture(hts);
        if (gesture != TS_GESTURE_NONE && hts->GestureCallback != NULL) {
            hts->GestureCallback(gesture);
        }
    }

    TS_WriteRegister(hts, STMPE811_REG_INT_STA, intStatus);
}

void TS_ServiceIRQ(void)
{
    TS_HandleTypeDef *hts = TS_GetHandle();

    if (s_irqPending && TS_CheckReady(hts) == TS_OK) {
        s_irqPending = false;
        TS_IRQHandler(hts);
    }
}

/**
 * @brief EXTI line callback for the touch interrupt pin
 * @param GPIO_Pin Pin that raised the edge
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin != TS_INT_PIN) {
        return;
    }

    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    APP_TouchActivity();

    /* Clearing the STMPE811 needs I2C, so defer it to TS_ServiceIRQ(). */
    s_irqPending = true;
}
