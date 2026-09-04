/**
 ******************************************************************************
 * @file    lv_port_indev.c
 * @brief   LVGL touchscreen driver — connects LVGL to the touch hardware
 * @details Reads touch coordinates from the STMPE811 that gui_board.c
 *          brought up and reports them to LVGL.
 ******************************************************************************
 */

#include <stdint.h>
#include "lvgl.h"
#include "lv_port_indev.h"
#include "gui_board.h"
#include "ts_touch.h"
#include "ts_events.h"
#include "app_low_power.h" /* Application low power management */

/* Last known touch position */
static int16_t last_x = 0;
static int16_t last_y = 0;
static bool touch_pressed = false;
/*-----------------------------------------------------------------------------
 * Touch Read Callback - Get Current Touch State
 *---------------------------------------------------------------------------*/
/* This function is called by LVGL periodically to check for touch input.
 *
 * Parameters:
 * - indev: Input device (v9 API)
 * - data:  Structure to fill with touch information
 *
 * What to implement:
 * 1. Read touch controller via I2C
 * 2. If touched: set data->state = LV_INDEV_STATE_PRESSED
 * 3. Set data->point.x and data->point.y to touch coordinates
 * 4. If not touched: set data->state = LV_INDEV_STATE_RELEASED
 */
/* LVGL input device read callback */
static void touch_read(lv_indev_t *indev, lv_indev_data_t *data) {
    (void)indev; /* Unused */

    TS_HandleTypeDef *hts = GUI_Board_Touch();
    uint16_t posX = 0;
    uint16_t posY = 0;

    if (!hts->IsInitialized) {
        data->state = LV_INDEV_STATE_RELEASED;
        data->point.x = last_x;
        data->point.y = last_y;
        return;
    }

    /* Fully interrupt-driven: only read touch on IRQ */
    if (!TS_IrqPending(hts)) {
        data->state = touch_pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
        data->point.x = last_x;
        data->point.y = last_y;
        return;
    }

    /* Service pending touchscreen IRQ (deferred from EXTI) */
    TS_ServiceIRQ(hts);

    if (TS_GetSingleTouch(hts, &posX, &posY) == TS_OK) {
        /* Clamp coordinates to LVGL display bounds */
        if (posX >= GUI_BOARD_DISPLAY_WIDTH) {
            posX = GUI_BOARD_DISPLAY_WIDTH - 1;
        }
        if (posY >= GUI_BOARD_DISPLAY_HEIGHT) {
            posY = GUI_BOARD_DISPLAY_HEIGHT - 1;
        }

        data->point.x = posX;
        data->point.y = posY;
        data->state = LV_INDEV_STATE_PRESSED;

        /* Clamped to the display size above, so the int16_t cast cannot truncate */
        touch_pressed = true;
        last_x = (int16_t)posX;
        last_y = (int16_t)posY;

        /* Update activity timestamp on touch */
        APP_TouchActivity();
    }
    else {
        touch_pressed = false;
        data->state = LV_INDEV_STATE_RELEASED;
        data->point.x = last_x;
        data->point.y = last_y;
    }
}

/*-----------------------------------------------------------------------------
 * Initialize Input Device Port
 *---------------------------------------------------------------------------*/
/* Call this function once during startup (after lv_init() and GUI_Board_Init()).
 * It registers the touchscreen with LVGL.
 */
/* LVGL input device initialization */
void lv_port_indev_init(void) {
    /* The board owns the controller; without it there is nothing to register. */
    if (!GUI_Board_Touch()->IsInitialized) {
        return;
    }

    /* Create LVGL input device driver */
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touch_read);
}
