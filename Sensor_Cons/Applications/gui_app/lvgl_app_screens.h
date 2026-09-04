/**
 ******************************************************************************
 * @file    lvgl_app_screens.h
 * @brief   Screen registry — maps screen IDs to live screen objects
 * @details Keeps a table of screen ID -> lv_obj_t so screens can find each
 *          other without caring about creation order.
 ******************************************************************************
 */

#ifndef LVGL_APP_SCREENS_H
#define LVGL_APP_SCREENS_H

#include "lvgl.h"
#include "lvgl_app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register a screen object under an ID
 * @param screenId  Screen ID
 * @param screen    LVGL screen object
 */
void LVGL_Screens_Register(LVGL_ScreenId_t screenId, lv_obj_t *screen);

/**
 * @brief Get a registered screen by ID
 * @param screenId  Screen ID
 * @retval lv_obj_t* The screen object, or NULL if not yet created
 */
lv_obj_t *LVGL_Screens_Get(LVGL_ScreenId_t screenId);

/**
 * @brief Clear all registered screens (called on re-init)
 */
void LVGL_Screens_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* LVGL_APP_SCREENS_H */
