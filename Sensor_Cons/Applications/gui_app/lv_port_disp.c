/**
 * @file    lv_port_disp.c
 * @brief   LVGL Display Port for STM32F429I-DISC1 (LVGL v9 API)
 * @details Connects LVGL to ILI9341 LCD via LTDC with SDRAM framebuffer.
 *          Based on ST BSP approach - direct SDRAM access, simple and reliable.
 * @version 2.0
 * @date    2026-01-03
 */

#include "lv_port_disp.h"
#include "gui_board.h"
#include "lvgl.h"
#include <stdint.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "cachel1_armv7.h"
#include "log.h"

/*-----------------------------------------------------------------------------
 * Display Configuration (all board facts come from gui_board.h)
 *---------------------------------------------------------------------------*/

#define DISP_HOR_RES GUI_BOARD_DISPLAY_WIDTH
#define DISP_VER_RES GUI_BOARD_DISPLAY_HEIGHT
#define DISP_BPP GUI_BOARD_BYTES_PER_PIXEL
#define FB_BASE_ADDR GUI_BOARD_FRAMEBUFFER_ADDR
#define FB_SIZE GUI_BOARD_FRAMEBUFFER_SIZE

/*-----------------------------------------------------------------------------
 * Draw Buffer Configuration
 *---------------------------------------------------------------------------*/

/** Number of lines to buffer for partial rendering (saves internal RAM) */
#define DRAW_BUF_LINES 40

/** Draw buffer size */
#define DRAW_BUF_SIZE (DISP_HOR_RES * DRAW_BUF_LINES)

/** Draw buffer in internal RAM (not SDRAM) */
static lv_color_t draw_buf[DRAW_BUF_SIZE] __attribute__((aligned(4)));

/*-----------------------------------------------------------------------------
 * Private Variables
 *---------------------------------------------------------------------------*/

/**
 * @brief   LVGL flush callback - copy rendered pixels to SDRAM framebuffer
 * @param   disp    Display object
 * @param   area    Area to update (x1,y1 to x2,y2)
 * @param   px_map  Pixel data from LVGL
 */
static void disp_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    /* Return if the area is completely outside the screen */
    if (area->x2 < 0) {
        return;
    }
    if (area->y2 < 0) {
        return;
    }
    if (area->x1 > DISP_HOR_RES - 1) {
        return;
    }
    if (area->y1 > DISP_VER_RES - 1) {
        return;
    }

    /* Truncate the area to the visible screen */
    int32_t act_x1 = area->x1 < 0 ? 0 : area->x1;
    int32_t act_y1 = area->y1 < 0 ? 0 : area->y1;
    int32_t act_x2 = area->x2 > DISP_HOR_RES - 1 ? DISP_HOR_RES - 1 : area->x2;
    int32_t act_y2 = area->y2 > DISP_VER_RES - 1 ? DISP_VER_RES - 1 : area->y2;

    uint16_t *frameBuf = (uint16_t *)FB_BASE_ADDR;

    /* Source width (pixels) of the original px_map lines */
    int32_t src_w = area->x2 - area->x1 + 1;

    /* Number of pixels to skip at the left/top of the px_map when clipped */
    int32_t skip_x = act_x1 - area->x1;
    int32_t skip_y = act_y1 - area->y1;

    /* Starting source pointer after skipping clipped pixels/lines */
    uint16_t *src = (uint16_t *)px_map + (skip_y * src_w) + skip_x;

    /* Width (pixels) actually written per line */
    int32_t lineWidth = act_x2 - act_x1 + 1;

    for (int32_t row = act_y1; row <= act_y2; row++) {
        memcpy(&frameBuf[row * DISP_HOR_RES + act_x1], src, (size_t)(lineWidth * DISP_BPP));
        src += src_w; /* advance to next source line */
    }

    /* Ensure LTDC sees updated SDRAM content for the written rectangle */
    uint32_t *clean_addr = (uint32_t *)&frameBuf[act_y1 * DISP_HOR_RES + act_x1];
    int32_t clean_size = (int32_t)(lineWidth * DISP_BPP * (act_y2 - act_y1 + 1));
    SCB_CleanDCache_by_Addr(clean_addr, clean_size);

    lv_display_flush_ready(disp);
}
/*-----------------------------------------------------------------------------
 * Public Functions
 *---------------------------------------------------------------------------*/

/**
 * @brief   Initialize LVGL display driver
 * @note    Call after lv_init(), SDRAM init, ILI9341 init, and LTDC init
 */
void lv_port_disp_init(void) {
    log_debug("LVGL: Initializing display port");
    /* Clear framebuffer */
    memset((void *)FB_BASE_ADDR, 0x00, FB_SIZE);

    /* Point LTDC to framebuffer */
    HAL_LTDC_SetAddress(GUI_Board_Ltdc(), FB_BASE_ADDR, 0);

    /* Create LVGL display (v9 API) */
    lv_display_t *disp = lv_display_create(DISP_HOR_RES, DISP_VER_RES);
    if (!disp) {
        log_error("LVGL: Failed to create display");
        while (1) {
            ;
        }
    }

    /* Set flush callback */
    lv_display_set_flush_cb(disp, disp_flush_cb);

    /* Configure partial draw buffer */
    lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

    /* Set color format */
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    log_debug("LVGL: Display port initialized");
}
