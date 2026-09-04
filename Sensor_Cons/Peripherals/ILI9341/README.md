Peripherals/ILI9341
====================

ILI9341 driver adapted from ST's BSP and organized by responsibility:

- `ili9341.h` is the public API aggregator.
- `ili9341_types.h` owns controller constants, timing values, and the `ILI9341_Config_t` / `ILI9341_Handle_t` types.
- `ili9341.c` and `ili9341_core.h` own panel lifecycle, sleep/display controls, and the ordered initialization sequence.
- `ili9341_io.c` and `ili9341_io.h` own the CS/DC/RST lines and the SPI command, data, and read transactions.

The table-driven initialization sequence preserves the ST command order and its vendor-specified parameter bytes while keeping the lifecycle code focused on panel setup.

Wiring is decided by the application. It opens the SPI bus (which owns SCK/MISO/MOSI) and tells the driver which bus and which CS/DC/RST pins the panel sits on; the driver configures those three lines itself. Leave `rstPort` NULL when the panel's reset is not under software control, as on the STM32F429I-DISC1 where it is tied to NRST.

Usage example (STM32F429I-DISC1 on-board panel):

    #include "spi.h"
    #include "ili9341.h"

    SPI_BusConfig_t busConfig = {
        .instance = SPI5,
        .sckPort = GPIOF, .sckPin = GPIO_PIN_7,
        .misoPort = GPIOF, .misoPin = GPIO_PIN_8,
        .mosiPort = GPIOF, .mosiPin = GPIO_PIN_9,
    };
    SPI_Bus_t bus;
    SPI_BusInit(&bus, &busConfig);

    ILI9341_Config_t lcdConfig = {
        .bus = &bus,
        .csPort = GPIOC, .csPin = GPIO_PIN_2,
        .dcPort = GPIOD, .dcPin = GPIO_PIN_13,
        .rstPort = NULL,
    };
    ILI9341_Handle_t lcd;

    ILI9341_Init(&lcd, &lcdConfig);
    uint16_t id = ILI9341_ReadID(&lcd);
    ILI9341_DisplayOn(&lcd);
