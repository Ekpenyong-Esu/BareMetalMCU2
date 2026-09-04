/**
 ******************************************************************************
 * @file    ssd1306_cmd.h
 * @brief   SSD1306 command opcodes and register values
 ******************************************************************************
 */

#ifndef SSD1306_CMD_H
#define SSD1306_CMD_H

/* Control byte prefixed to every I2C transfer */
#define SSD1306_CTRL_COMMAND 0x00
#define SSD1306_CTRL_DATA 0x40

/* Fundamental commands */
#define SSD1306_CMD_SET_CONTRAST 0x81
#define SSD1306_CMD_DISPLAY_ALL_ON_RESUME 0xA4
#define SSD1306_CMD_DISPLAY_ALL_ON 0xA5
#define SSD1306_CMD_NORMAL_DISPLAY 0xA6
#define SSD1306_CMD_INVERT_DISPLAY 0xA7
#define SSD1306_CMD_DISPLAY_OFF 0xAE
#define SSD1306_CMD_DISPLAY_ON 0xAF

/* Addressing */
#define SSD1306_CMD_SET_LOW_COLUMN 0x00
#define SSD1306_CMD_SET_HIGH_COLUMN 0x10
#define SSD1306_CMD_MEMORY_MODE 0x20
#define SSD1306_CMD_COLUMN_ADDR 0x21
#define SSD1306_CMD_PAGE_ADDR 0x22

/* Hardware configuration */
#define SSD1306_CMD_SET_START_LINE 0x40
#define SSD1306_CMD_SET_MULTIPLEX 0xA8
#define SSD1306_CMD_SEG_REMAP 0xA0
#define SSD1306_CMD_COM_SCAN_INC 0xC0
#define SSD1306_CMD_COM_SCAN_DEC 0xC8
#define SSD1306_CMD_SET_DISPLAY_OFFSET 0xD3
#define SSD1306_CMD_SET_COM_PINS 0xDA

/* Timing and driving scheme */
#define SSD1306_CMD_SET_DISPLAY_CLOCK_DIV 0xD5
#define SSD1306_CMD_SET_PRECHARGE 0xD9
#define SSD1306_CMD_SET_VCOM_DETECT 0xDB
#define SSD1306_CMD_CHARGE_PUMP 0x8D

/* Parameter values used by the 128x64 bring-up sequence */
#define SSD1306_CLOCK_DIV_DEFAULT 0x80 /*!< Divide ratio 1, oscillator 8 */
#define SSD1306_MULTIPLEX_64 0x3F      /*!< 64 COM lines */
#define SSD1306_DISPLAY_OFFSET_NONE 0x00
#define SSD1306_CHARGE_PUMP_ENABLE 0x14
#define SSD1306_CHARGE_PUMP_DISABLE 0x10
#define SSD1306_MEMORY_MODE_HORIZONTAL 0x00
#define SSD1306_MEMORY_MODE_VERTICAL 0x01
#define SSD1306_MEMORY_MODE_PAGE 0x02
#define SSD1306_SEG_REMAP_FLIP 0x01 /*!< OR into SEG_REMAP */
#define SSD1306_COM_PINS_ALTERNATIVE 0x12
#define SSD1306_CONTRAST_DEFAULT 0xCF
#define SSD1306_PRECHARGE_DEFAULT 0xF1
#define SSD1306_VCOM_LEVEL_0_77 0x40

#endif /* SSD1306_CMD_H */
