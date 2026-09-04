/**
 ******************************************************************************
 * @file    lvgl_app.c
 * @brief   LVGL app — builds the four screens and handles updates
 * @details Creates the home, sensors, settings and info screens, then
 *          forwards update calls (like SetTemperature) to the right screen.
 ******************************************************************************
 */

#include "lvgl.h"
#include "lvgl_app_core.h"
#include "lvgl_app_screens.h"
#include "lvgl_app_home.h"
#include "lvgl_app_sensors.h"
#include "lvgl_app_settings.h"
#include "lvgl_app_info.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "gui_board.h"
#include "log.h"

static bool s_initialized = false;

void LVGL_App_Init(void) {
    lv_obj_t *home = NULL;

    /* Re-running would build a second set of screens and register a second
       display and input device, leaking the first of each. */
    if (s_initialized) {
        return;
    }

    log_debug("LVGL: Initializing application");

    /* The ports draw into SDRAM and read the touch handle, so the board must
       be up before either of them registers with LVGL. */
    if (!GUI_Board_Init()) {
        log_error("LVGL: board bring-up failed; no display");
        return;
    }

    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();

    LVGL_Screens_Reset();

    /* Order is free: buttons resolve their target through the registry on click. */
    LVGL_Home_Create();
    LVGL_Sensors_Create();
    LVGL_Settings_Create();
    LVGL_Info_Create();

    home = LVGL_Screens_Get(LVGL_SCREEN_HOME);
    if (home == NULL) {
        log_error("LVGL: home screen was not created");
        return;
    }

    lv_screen_load(home);
    lv_refr_now(NULL);

    s_initialized = true;
    log_debug("LVGL: Application initialized successfully");
}

void LVGL_App_Tick(void) {
    if (s_initialized) {
        lv_timer_handler();
    }
}

bool LVGL_App_IsInitialized(void) {
    return s_initialized;
}

void LVGL_App_UpdateTemperature(int temp_celsius) {
    LVGL_Home_SetTemperature(temp_celsius);
}

void LVGL_App_UpdateHumidity(int humidity_percent) {
    LVGL_Home_SetHumidity(humidity_percent);
}

void LVGL_App_UpdateStatus(const char *status) {
    LVGL_Home_SetStatus(status);
}

void LVGL_App_AddChartData(int value) {
    LVGL_Sensors_AddChartData(value);
}
