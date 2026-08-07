/**
 * @file ltdc_events.c
 * @brief LTDC HAL callback overrides
 * @details The controller is a singleton, so a single attached driver record
 *          receives the events.
 */

#include "ltdc_events.h"

static LTDC_Driver_t *s_driver = NULL;   /*!< Driver record receiving HAL callbacks */

void LTDC_Events_Attach(LTDC_Driver_t *driver)
{
    s_driver = driver;
}

void LTDC_Events_Detach(LTDC_Driver_t *driver)
{
    if (s_driver == driver) {
        s_driver = NULL;
    }
}

/**
 * @brief Shadow register reload completed (VSYNC)
 * @param hltdc HAL handle that raised the event
 */
void HAL_LTDC_ReloadEventCallback(LTDC_HandleTypeDef *hltdc)
{
    (void)hltdc;

    if (s_driver != NULL) {
        s_driver->reloadFlag = 1;
        __SEV();    /* Wake the waiter parked in __WFE() inside LTDC_WaitForReload() */
    }
}
