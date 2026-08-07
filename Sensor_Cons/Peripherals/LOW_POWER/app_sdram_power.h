/**
  ******************************************************************************
  * @file    app_sdram_power.h
  * @brief   External SDRAM power state and re-initialization
  * @note    Internal to the low power manager; not part of app_low_power.h.
  ******************************************************************************
  */

#ifndef APP_SDRAM_POWER_H
#define APP_SDRAM_POWER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_low_power_types.h"

/**
 * @brief Disable the FMC clock and pins backing the SDRAM.
 * @note  Every framebuffer access after this faults; power the display down first.
 */
void APP_SDRAM_PowerOff(void);

/**
 * @brief Restore the FMC pins and re-run the SDRAM initialization sequence.
 * @retval false if the controller could not be re-initialized.
 */
bool APP_SDRAM_PowerOn(void);

bool APP_SDRAM_IsActive(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_SDRAM_POWER_H */
