Peripherals/ILI9341
====================

ILI9341 driver adapted from ST's BSP and organized by responsibility:

- `ili9341.h` is the public API aggregator.
- `ili9341_types.h` owns controller constants, timing values, and default board pins.
- `ili9341.c` and `ili9341_core.h` own panel lifecycle, sleep/display controls, and the ordered initialization sequence.
- `ili9341_io.c` and `ili9341_io.h` own SPI command, data, and read transactions.
- `ili9341_board.c` owns STM32F429I-DISC1 GPIO and SPI5 configuration through `ILI9341_MspInit()` and `ILI9341_MspDeInit()`.

The table-driven initialization sequence preserves the ST command order and its vendor-specified parameter bytes while keeping the lifecycle code focused on panel setup.

Usage example:

    #include "ili9341.h"

    ili9341_Init();
    uint16_t id = ili9341_ReadID();
    ili9341_DisplayOn();
