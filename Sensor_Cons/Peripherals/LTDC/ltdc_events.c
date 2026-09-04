/**
 * @file ltdc_events.c
 * @brief LTDC HAL callback overrides and interrupt entry
 * @details The controller is a singleton, so a single attached driver record
 *          receives the events. The HAL handle itself belongs to the
 *          application; only this pointer to the record is kept here.
 */

#include "ltdc_events.h"
#include "ltdc_core.h"

static LTDC_Driver_t *s_driver = NULL; /*!< Driver record receiving HAL callbacks */

void LTDC_Events_Attach(LTDC_Driver_t *driver) {
    s_driver = driver;
}

void LTDC_Events_Detach(LTDC_Driver_t *driver) {
    if (s_driver == driver) {
        s_driver = NULL;
    }
}

void LTDC_ISR_Dispatch(void) {
    if (s_driver != NULL && s_driver->hltdc != NULL) {
        HAL_LTDC_IRQHandler(s_driver->hltdc);
    }
}

/**
 * @brief Shadow register reload completed (VSYNC)
 * @param hltdc HAL handle that raised the event
 */
void HAL_LTDC_ReloadEventCallback(LTDC_HandleTypeDef *hltdc) {
    (void)hltdc;

    if (s_driver != NULL) {
        s_driver->reloadFlag = 1;
        __SEV(); /* Wake the waiter parked in __WFE() inside LTDC_WaitForReload() */
    }
}
