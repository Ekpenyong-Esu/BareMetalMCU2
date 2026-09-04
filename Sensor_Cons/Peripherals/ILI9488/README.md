# ILI9488 TFT LCD Display Driver

This driver provides support for 4-inch TFT LCD displays with ILI9488 controller.

## Features

- 320x480 resolution (configurable orientation)
- 16-bit RGB565 color support
- SPI interface
- Hardware reset and chip select control
- Basic graphics functions (pixels, lines, rectangles, circles)
- Text rendering with 6x8 font
- Multiple orientation support

## Hardware Requirements

- ILI9488 TFT LCD display
- An SPI bus opened by the application (`SPI_BusInit`)
- 3 GPIO pins (CS, DC, RST), chosen by the application

## Usage Example

```c
#include "ili9488.h"

// The application owns the bus and decides which pins carry it
SPI_BusConfig_t busConfig = { .instance = SPI2,
                              .sckPort = GPIOB, .sckPin = GPIO_PIN_10,
                              .misoPort = GPIOC, .misoPin = GPIO_PIN_2,
                              .mosiPort = GPIOC, .mosiPin = GPIO_PIN_3 };
SPI_Bus_t bus;
SPI_BusInit(&bus, &busConfig);

// Initialize display on that bus; the driver configures CS/DC/RST itself
ILI9488_Handle_t hili;
ILI9488_Init(&hili, &bus, GPIOB, GPIO_PIN_12, GPIOB, GPIO_PIN_13, GPIOB, GPIO_PIN_14);

// Clear screen
ILI9488_Clear(&hili, ILI9488_COLOR_BLACK);

// Draw some graphics
ILI9488_DrawPixel(&hili, 100, 100, ILI9488_COLOR_RED);

// Write text
ILI9488_SetCursor(&hili, 10, 10);
ILI9488_WriteChar(&hili, 'A', ILI9488_COLOR_WHITE, ILI9488_COLOR_BLACK);
```

## Pin Configuration

| Pin | Function | Description |
|-----|----------|-------------|
| CS  | Chip Select | Active low |
| DC  | Data/Command | High for data, low for command |
| RST | Reset | Active low reset |

## CMake Integration

Enable the driver by setting `USE_ILI9488=ON` in CMakeLists.txt or command line:

```bash
cmake -DUSE_ILI9488=ON ..
```

## Dependencies

- `Peripherals/SPI` bus driver
- GPIO driver
- Standard C libraries
