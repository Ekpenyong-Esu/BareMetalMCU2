/**
  ******************************************************************************
  * @file    ssd1331_cmd.h
  * @brief   SSD1331 command opcodes and parameter values
  ******************************************************************************
  */

#ifndef SSD1331_CMD_H
#define SSD1331_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup SSD1331_Fundamental Fundamental commands
 * @{
 */
#define SSD1331_CMD_SET_COLUMN            0x15U  /**< + start, end */
#define SSD1331_CMD_SET_ROW               0x75U  /**< + start, end */
#define SSD1331_CMD_SET_CONTRAST_A        0x81U  /**< + level (red)   */
#define SSD1331_CMD_SET_CONTRAST_B        0x82U  /**< + level (green) */
#define SSD1331_CMD_SET_CONTRAST_C        0x83U  /**< + level (blue)  */
#define SSD1331_CMD_MASTER_CURRENT        0x87U  /**< + 0..15 */
#define SSD1331_CMD_PRECHARGE_A           0x8AU
#define SSD1331_CMD_PRECHARGE_B           0x8BU
#define SSD1331_CMD_PRECHARGE_C           0x8CU
#define SSD1331_CMD_SET_REMAP             0xA0U  /**< + remap/colour-depth byte */
#define SSD1331_CMD_SET_START_LINE        0xA1U  /**< + line */
#define SSD1331_CMD_SET_DISPLAY_OFFSET    0xA2U  /**< + offset */
#define SSD1331_CMD_NORMAL_DISPLAY        0xA4U
#define SSD1331_CMD_DISPLAY_ALL_ON        0xA5U
#define SSD1331_CMD_DISPLAY_ALL_OFF       0xA6U
#define SSD1331_CMD_INVERT_DISPLAY        0xA7U
#define SSD1331_CMD_SET_MULTIPLEX         0xA8U  /**< + ratio - 1 */
#define SSD1331_CMD_SET_MASTER_CONFIG     0xADU  /**< + master config byte */
#define SSD1331_CMD_DISPLAY_OFF           0xAEU
#define SSD1331_CMD_DISPLAY_ON            0xAFU
#define SSD1331_CMD_POWER_SAVE            0xB0U  /**< + mode */
#define SSD1331_CMD_PHASE_PERIOD          0xB1U  /**< + phase 2 : phase 1 */
#define SSD1331_CMD_SET_CLOCK_DIV         0xB3U  /**< + Fosc : divider */
#define SSD1331_CMD_SET_PRECHARGE_LEVEL   0xBBU  /**< + level */
#define SSD1331_CMD_SET_VCOMH             0xBEU  /**< + level */
#define SSD1331_CMD_DEACTIVATE_SCROLL     0x2EU
/** @} */

/** @defgroup SSD1331_Parameters Parameter values used by the bring-up sequence
 * @{
 */
/** Column remap, COM remap, COM split, 65k colour format 1 (RGB565, RGB order).
 *  Set bit 2 for panels wired BGR. */
#define SSD1331_REMAP_RGB565              0x72U
#define SSD1331_REMAP_BGR565              0x76U

#define SSD1331_START_LINE_NONE           0x00U
#define SSD1331_DISPLAY_OFFSET_NONE       0x00U
#define SSD1331_MULTIPLEX_64              0x3FU
#define SSD1331_MASTER_CONFIG_EXT_VCC     0x8EU
#define SSD1331_POWER_SAVE_DISABLE        0x0BU
#define SSD1331_PHASE_PERIOD_DEFAULT      0x31U
#define SSD1331_CLOCK_DIV_DEFAULT         0xF0U
#define SSD1331_PRECHARGE_SPEED_DEFAULT   0x64U
#define SSD1331_PRECHARGE_LEVEL_DEFAULT   0x3AU
#define SSD1331_VCOMH_DEFAULT             0x3EU
#define SSD1331_MASTER_CURRENT_DEFAULT    0x06U
#define SSD1331_CONTRAST_A_DEFAULT        0x91U
#define SSD1331_CONTRAST_B_DEFAULT        0x50U
#define SSD1331_CONTRAST_C_DEFAULT        0x7DU
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* SSD1331_CMD_H */
