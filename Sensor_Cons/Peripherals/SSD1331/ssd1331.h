/**
  ******************************************************************************
  * @file    ssd1331.h
  * @brief   SSD1331 0.95" 96x64 colour OLED driver - public aggregator
  * @details Four-wire SPI panel. The bus is the shared SPI driver; the three
  *          control lines are given per display, so the driver stays free of
  *          board wiring.
  *
  * Example
  * -------
  *   SSD1331_Handle_t oled;
  *   const SSD1331_Config_t config = {
  *       .csPort = GPIOC, .csPin = GPIO_PIN_4,
  *       .dcPort = GPIOC, .dcPin = GPIO_PIN_5,
  *       .rstPort = GPIOB, .rstPin = GPIO_PIN_1
  *   };
  *
  *   SSD1331_Init(&oled, &config);
  *   SSD1331_SetCursor(&oled, 0, 0);
  *   SSD1331_WriteString(&oled, "Hello", SSD1331_COLOR_CYAN, SSD1331_COLOR_BLACK);
  *   SSD1331_UpdateScreen(&oled);
  ******************************************************************************
  */

#ifndef SSD1331_H
#define SSD1331_H

#include "ssd1331_types.h"
#include "ssd1331_cmd.h"
#include "ssd1331_core.h"
#include "ssd1331_buffer.h"
#include "ssd1331_text.h"
#include "ssd1331_font.h"

#endif /* SSD1331_H */
