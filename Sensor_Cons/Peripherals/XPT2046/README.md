# XPT2046 Resistive Touchscreen Driver

This driver provides support for resistive touchscreens with XPT2046 controller.

## Features

- 12-bit ADC resolution (4096 x 4096 points)
- Touch pressure detection
- SPI interface
- Interrupt-driven touch detection
- Coordinate calibration and mapping
- Debounced touch state tracking

## Hardware Requirements

- XPT2046 resistive touchscreen controller
- SPI peripheral
- 2 GPIO pins (CS, IRQ)

## Usage Example

```c
#include "xpt2046.h"

// Initialize touchscreen
XPT2046_Handle_t hxpt;
XPT2046_Init(&hxpt, GPIOB, GPIO_PIN_15, GPIOB, GPIO_PIN_0, 320, 480);

// Check for touch
if (XPT2046_IsTouched(&hxpt)) {
    XPT2046_TouchPoint_t touch;
    if (XPT2046_ReadTouch(&hxpt, &touch) == XPT2046_OK) {
        // Use touch coordinates
        uint16_t x = touch.x;
        uint16_t y = touch.y;
        uint16_t pressure = touch.pressure;
    }
}

// Or use update method for state tracking
XPT2046_Update(&hxpt);
XPT2046_TouchPoint_t current_touch;
XPT2046_ReadTouch(&hxpt, &current_touch);
```

## Pin Configuration

| Pin | Function | Description |
|-----|----------|-------------|
| CS  | Chip Select | Active low |
| IRQ | Interrupt | Active low when touched |

## Calibration

A resistive panel never reaches the ADC rails, so the driver maps from a measured raw
span rather than from 0..4095. `XPT2046_Init` seeds a typical span; replace it with the
raw values you read at two opposite corners:

```c
XPT2046_SetCalibration(&hxpt, raw_x_min, raw_x_max, raw_y_min, raw_y_max);
```

Drawing the calibration crosshairs and storing the result belong to the application.

## Touch States

- `XPT2046_STATE_RELEASED`: No touch
- `XPT2046_STATE_PRESSED`: New touch detected
- `XPT2046_STATE_HELD`: Continued touch

## CMake Integration

Enable the driver by setting `USE_XPT2046=ON` in CMakeLists.txt or command line:

```bash
cmake -DUSE_XPT2046=ON ..
```

## Dependencies

- STM32 HAL SPI driver
- GPIO driver
- Standard C libraries
