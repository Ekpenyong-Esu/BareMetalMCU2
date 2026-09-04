# Nokia 5110 LCD Driver

This folder contains the driver for the Nokia 5110 LCD display (84x48 pixels) for the STM32F429 Discovery board.

## Features

- 84x48 pixel monochrome LCD display
- SPI interface communication
- Built-in 5x7 ASCII font
- Drawing primitives (pixels, lines, rectangles, circles)
- Text rendering
- Configurable contrast and display modes
- Buffer-based drawing for smooth updates

## Hardware Connections

The driver has no fixed wiring. The application opens an `SPI_Bus_t` for
DIN/CLK and names the three control lines in `NOKIA5110_Config_t`:

| Nokia 5110 Pin | Config field        | Description |
|----------------|---------------------|-------------|
| RST           | `RstPort`/`RstPin`  | Reset; `RstPort = NULL` if the module resets itself |
| CE            | `CePort`/`CePin`    | Chip Enable (SPI CS) |
| DC            | `DcPort`/`DcPin`    | Data/Command select |
| DIN, CLK      | `Bus`               | MOSI/SCK of the bus opened with `SPI_BusInit` |
| VCC           | 3.3V                | Power supply |
| GND           | GND                 | Ground |
| BL            | 3.3V/NC             | Backlight (optional) |

## Usage Example

```c
#include "nokia5110.h"
#include "spi.h"

SPI_Bus_t bus;                     // owned by the application
NOKIA5110_Handle_t hnok;

SPI_BusInit(&bus, &busConfig);     // e.g. SPI5 on PF7/PF9

NOKIA5110_Config_t config = NOKIA5110_GetDefaultConfig();
config.Bus = &bus;
config.RstPort = GPIOB; config.RstPin = GPIO_PIN_1;
config.CePort  = GPIOB; config.CePin  = GPIO_PIN_0;
config.DcPort  = GPIOB; config.DcPin  = GPIO_PIN_2;

// Initialize the LCD
if (NOKIA5110_Init(&hnok, &config) == NOKIA5110_OK) {
    // Draw some text
    NOKIA5110_DrawText(&hnok, 0, 0, "Hello World!", 1);

    // Draw a rectangle
    NOKIA5110_DrawRect(&hnok, 10, 10, 30, 20, 1);

    // Update the display
    NOKIA5110_Update(&hnok);
}
```

## API Reference

### Initialization and Configuration

- `NOKIA5110_Init()` - Initialize the LCD
- `NOKIA5110_DeInit()` - Deinitialize the LCD
- `NOKIA5110_Config()` - Configure LCD parameters
- `NOKIA5110_SetContrast()` - Set display contrast
- `NOKIA5110_SetMode()` - Set display mode

### Display Control

- `NOKIA5110_Clear()` - Clear the display
- `NOKIA5110_Update()` - Update display with buffer contents

### Drawing Functions

- `NOKIA5110_DrawPixel()` - Draw a single pixel
- `NOKIA5110_DrawLine()` - Draw a line
- `NOKIA5110_DrawRect()` - Draw a rectangle outline
- `NOKIA5110_FillRect()` - Draw a filled rectangle
- `NOKIA5110_DrawCircle()` - Draw a circle
- `NOKIA5110_DrawText()` - Draw text

### Utility Functions

- `NOKIA5110_GetWidth()` - Get display width
- `NOKIA5110_GetHeight()` - Get display height
- `NOKIA5110_GetDefaultConfig()` - Get default configuration

## Dependencies

- SPI peripheral driver (`Peripherals/SPI/`)
- GPIO peripheral driver (`Peripherals/GPIO/`)

## Notes

- The bus may be shared with other devices; the driver registers its own
  `SPI_Device_t` inside the handle, so several panels can coexist
- Open the bus with `SPI_BusInit` before calling `NOKIA5110_Init`
- The display buffer is 84x6 bytes (504 bytes total)
- Text rendering uses a built-in 5x7 ASCII font
- All drawing operations modify the buffer; call `NOKIA5110_Update()` to refresh the display
