/**
 * @file ltdc_events.h
 * @brief Routing of LTDC HAL callbacks and the interrupt to a driver record
 */

#ifndef LTDC_EVENTS_H
#define LTDC_EVENTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ltdc_types.h"

/**
 * @brief Route LTDC HAL callbacks and LTDC_ISR_Dispatch() to this driver record
 * @param driver Driver record, or NULL to route nowhere
 */
void LTDC_Events_Attach(LTDC_Driver_t *driver);

/**
 * @brief Stop routing callbacks to a driver record
 * @param driver Driver record; ignored if it is not the attached one
 */
void LTDC_Events_Detach(LTDC_Driver_t *driver);

#ifdef __cplusplus
}
#endif

#endif /* LTDC_EVENTS_H */
