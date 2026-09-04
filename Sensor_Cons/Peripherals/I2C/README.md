# I2C Driver

A bus-and-device model for the STM32F4 I2C peripherals. The application owns
the bus and says which peripheral and pins carry it. Device drivers register
on that bus with their address and the speed their chip needs. The peripheral
is reprogrammed whenever a different device is selected, so a 100 kHz sensor
and a 400 kHz display can share the same two wires.

## Files

| File | Holds |
|------|-------|
| `i2c_types.h` | `I2C_BusConfig_t`, `I2C_Bus_t`, `I2C_Device_t`, `I2C_ConfigTypeDef`, status codes |
| `i2c_core.[ch]` | Bus open/close, pin and clock setup, device registration, bus ownership |
| `i2c_transfer.[ch]` | Master transmit/receive, register (memory) read/write, probe, scan, stuck-bus recovery |
| `i2c.h` | Umbrella include |

## Who owns what

| Owner | Owns | Type |
|-------|------|------|
| Application | The peripheral (I2C1..I2C3), SCL/SDA pins | `I2C_Bus_t` |
| Device driver | The chip's address and bus speed | `I2C_Device_t` inside its handle |

The driver never picks an instance or a pin on its own. Nothing in this
directory names a board or includes `main.h`.

## Opening a bus (application code)

```c
#include "i2c.h"

static I2C_Bus_t s_i2c3;

const I2C_BusConfig_t wiring = {
    .instance  = I2C3,
    .sclPort   = GPIOA, .sclPin = GPIO_PIN_8,
    .sdaPort   = GPIOC, .sdaPin = GPIO_PIN_9,
    .alternate = 0,     /* 0 means AF4, which every I2C pin uses bar a few */
};

if (I2C_BusInit(&s_i2c3, &wiring) != I2C_OK) {
    /* wrong instance or missing pin */
}
```

`I2C_BusInit` enables and resets the peripheral clock and configures both
pins as open-drain alternate function with pull-ups. It does not program the
peripheral yet: the speed belongs to whichever device is selected first.

A handful of pins use a different alternate function (PB4 as I2C3_SDA is
AF9). Pass the right `GPIO_AFx_I2Cx` in `alternate` for those.

## Registering a device (driver code)

```c
typedef struct {
    I2C_Device_t device;
} MYSENSOR_Handle_t;

MYSENSOR_Status MYSENSOR_Init(MYSENSOR_Handle_t *h, I2C_Bus_t *bus)
{
    I2C_ConfigTypeDef settings = I2C_ConfigDefault();  /* 100 kHz, 7-bit */
    settings.ClockSpeed = 400000U;

    /* HAL addresses are the 7-bit address shifted left by one. */
    if (I2C_DeviceInit(&h->device, bus, 0x48U << 1, &settings) != I2C_OK) {
        return MYSENSOR_ERROR;   /* bus not open */
    }
    ...
}
```

Transfers name the device. They select it onto the bus, then use its address:

```c
uint8_t who;
I2C_Mem_Read(&h->device, WHO_AM_I_REG, I2C_MEMADD_SIZE_8BIT, &who, 1, I2C_TIMEOUT_DEFAULT);
I2C_Master_Transmit(&h->device, payload, sizeof payload, I2C_TIMEOUT_DEFAULT);
```

`I2C_IsDeviceReady(&device, trials, timeout)` probes the chip's address.
`I2C_ScanBus(&device, list, max, timeout)` walks 0x08..0x77 on that device's
bus and fills `list` with the 7-bit addresses that answered.

## Error recovery

A NACK is a normal reply from an absent chip and comes back as `I2C_NACK`
without touching the bus. Any other failure resets the peripheral, bit-bangs
the configured SCL/SDA pins free (nine clocks then a STOP), puts the pins
back into alternate-function mode, and drops the bus owner. The next transfer
reprograms the peripheral. `I2C_GetError(&device)` returns the HAL error code
of the last transfer on that device's bus.

## Closing

`I2C_BusDeInit(&bus)` stops the peripheral, releases the pins, and gates the
clock. Devices stay registered; reopen the bus and they work again.

## Status codes

`I2C_OK`, `I2C_ERROR`, `I2C_BUSY`, `I2C_TIMEOUT`, `I2C_NACK`,
`I2C_INVALID_PARAM`. `I2C_GetStatusString()` describes them.
