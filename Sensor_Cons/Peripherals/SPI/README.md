# SPI Driver

A bus-and-device model for the STM32F4 SPI peripherals. The application owns
the bus and says which peripheral and pins carry it. Device drivers register
on that bus with the settings their chip needs. The peripheral is reprogrammed
whenever a different device is selected, so a slow chip and a fast chip can
share the same wires.

## Files

| File | Holds |
|------|-------|
| `spi_types.h` | `SPI_BusConfig_t`, `SPI_Bus_t`, `SPI_Device_t`, `SPI_ConfigTypeDef`, status codes |
| `spi_core.[ch]` | Bus open/close, pin and clock setup, device registration, bus ownership |
| `spi_transfer.[ch]` | Blocking transmit, receive, and full-duplex exchange |
| `spi.h` | Umbrella include |

## Who owns what

| Owner | Owns | Type |
|-------|------|------|
| Application | The peripheral (SPI1..SPI6), SCK/MISO/MOSI pins | `SPI_Bus_t` |
| Device driver | Chip select (and DC/RST/INT) pins, bus settings the chip needs | `SPI_Device_t` inside its handle |

The driver never picks an instance or a pin on its own. Nothing in this
directory names a board.

## Opening a bus (application code)

```c
#include "spi.h"

static SPI_Bus_t s_spi5;

const SPI_BusConfig_t wiring = {
    .instance  = SPI5,
    .sckPort   = GPIOF, .sckPin  = GPIO_PIN_7,
    .misoPort  = GPIOF, .misoPin = GPIO_PIN_8,
    .mosiPort  = GPIOF, .mosiPin = GPIO_PIN_9,
    .pull      = GPIO_PULLDOWN,   /* optional, GPIO_NOPULL when left 0 */
    .alternate = 0,               /* 0 derives the AF from the instance */
};

if (SPI_BusInit(&s_spi5, &wiring) != SPI_OK) {
    /* wrong instance, or no clock pin */
}
```

`SPI_BusInit` enables the peripheral clock and configures the pins. It does
not program the peripheral yet: the settings belong to whichever device is
selected first. A `NULL` MISO or MOSI port leaves that line alone, for
write-only displays or boards that set the pin up elsewhere.

The alternate function is derived from the instance (AF6 for SPI3, AF5 for
the rest). A few pins map differently, e.g. PD6 as SPI3_MOSI is AF5; pass the
right `GPIO_AFx_SPIx` in `alternate` for those.

## Registering a device (driver code)

```c
typedef struct {
    SPI_Device_t  device;
    GPIO_TypeDef *csPort;
    uint16_t      csPin;
} MYCHIP_Handle_t;

MYCHIP_Status MYCHIP_Init(MYCHIP_Handle_t *h, SPI_Bus_t *bus,
                          GPIO_TypeDef *csPort, uint16_t csPin)
{
    SPI_ConfigTypeDef settings = SPI_ConfigDefault();   /* mode 0, 8-bit, PCLK/8 */
    settings.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;

    if (SPI_DeviceInit(&h->device, bus, &settings) != SPI_OK) {
        return MYCHIP_ERROR;   /* bus not open */
    }
    /* configure csPort/csPin as an output here */
    ...
}
```

Transfers name the device, and select it onto the bus themselves:

```c
HAL_GPIO_WritePin(h->csPort, h->csPin, GPIO_PIN_RESET);
SPI_Transmit(&h->device, cmd, sizeof cmd, SPI_TIMEOUT_DEFAULT);
SPI_Receive(&h->device, reply, sizeof reply, SPI_TIMEOUT_DEFAULT);
HAL_GPIO_WritePin(h->csPort, h->csPin, GPIO_PIN_SET);
```

`SPI_Select` is only called by the transfer functions; it reprograms the
peripheral when ownership changes and is free otherwise. Two chips with
different settings on one bus therefore cost one `HAL_SPI_Init` per switch.

## Error recovery

A failed transfer resets the peripheral and drops its owner. The next
transfer, from any device, programs it again. `SPI_GetError(&device)` returns
the HAL error code of the last transfer on that device's bus.

## Closing

`SPI_BusDeInit(&bus)` stops the peripheral, releases the pins, and gates the
clock. Devices stay registered; reopen the bus and they work again.

## Status codes

`SPI_OK`, `SPI_ERROR`, `SPI_BUSY`, `SPI_TIMEOUT`, `SPI_INVALID_PARAM`.
`SPI_GetStatusString()` names them.
