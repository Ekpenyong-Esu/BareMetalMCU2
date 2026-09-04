# STMPE811 Touchscreen Driver

This driver provides an interface for the STMPE811 resistive touchscreen controller found on the STM32F429I Discovery board.

## Hardware Overview

The driver knows the STMPE811 register map and nothing about the board. The
application supplies the wiring in `TS_ConfigTypeDef`: the open `I2C_Bus_t`
the controller sits on, the MCU pin its INT output reaches (or none, for
polling) and the size of the display behind the panel.

On the STM32F429I Discovery board that is:
- **STMPE811** at 8-bit address `0x82` (the default when `address` is 0)
- **240x320 pixel** resistive touchscreen display
- **I2C3** on PA8 (SCL) / PC9 (SDA)
- **PA15** interrupt pin for touch detection

## Features

- ✅ Single-point touch detection
- ✅ Pressure sensitivity measurement
- ✅ Gesture recognition (tap, swipe, etc.)
- ✅ Calibration support
- ✅ Interrupt-driven operation
- ✅ FIFO buffer support
- ✅ Hardware filtering and averaging

## Quick Start

### 1. Basic Initialization

The application opens the bus, then hands it to the driver together with the
INT pin and display size:

```c
#include "i2c.h"
#include "ts_core.h"

I2C_Bus_t i2c3;
TS_HandleTypeDef hts;

I2C_BusConfig_t busCfg = {
    .instance = I2C3,
    .sclPort = GPIOA, .sclPin = GPIO_PIN_8,
    .sdaPort = GPIOC, .sdaPin = GPIO_PIN_9,
};
I2C_BusInit(&i2c3, &busCfg);

TS_ConfigTypeDef tsCfg = TS_GetDefaultConfig();
tsCfg.bus = &i2c3;
tsCfg.intPort = GPIOA;          /* NULL to poll instead of using EXTI */
tsCfg.intPin = GPIO_PIN_15;
tsCfg.displayWidth = 240;
tsCfg.displayHeight = 320;

if (TS_Init(&hts, &tsCfg) == TS_OK) {
    printf("Touchscreen initialized successfully\\n");
}
```

### 2. Touch Detection

```c
TS_TouchDataTypeDef touchData;

// Poll for touch data
if (TS_GetTouchData(&hts, &touchData) == TS_OK) {
    if (touchData.TouchCount > 0) {
        uint16_t x = touchData.Points[0].X;
        uint16_t y = touchData.Points[0].Y;
        uint16_t pressure = touchData.Points[0].Z;
        
        printf("Touch at (%d, %d) with pressure %d\\n", x, y, pressure);
    }
}
```

### 3. Interrupt-Driven Operation

The driver never defines `HAL_GPIO_EXTI_Callback`; the application owns it and
forwards the edge of the pin it configured. The EXTI callback only raises a
flag, because clearing the STMPE811 needs I2C, so the application services it
from its main loop or LVGL task.

```c
// Set callbacks
TS_RegisterCallbacks(&hts, MyTouchCallback, MyReleaseCallback, MyGestureCallback);
TS_SetActivityCallback(&hts, MyWakeBookkeeping);   /* optional, runs in ISR context */

// In your interrupt handler (stm32f4xx_it.c)
void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == tsCfg.intPin) {
        TS_EXTI_Callback(&hts);
    }
}

// In the main loop / LVGL task
if (TS_IrqPending(&hts)) {
    TS_ServiceIRQ(&hts);
}

// Callback functions
void MyTouchCallback(void) {
    printf("Touch detected!\\n");
}

void MyGestureCallback(TS_GestureTypeDef gesture) {
    switch (gesture) {
        case TS_GESTURE_TAP:
            printf("Tap gesture\\n");
            break;
        case TS_GESTURE_SWIPE_UP:
            printf("Swipe up gesture\\n");
            break;
        // Handle other gestures...
    }
}
```

### 4. Calibration

`TS_Init` seeds typical raw bounds. To calibrate, have the user touch two opposite
corners, record the raw ADC values and hand them to the driver:

```c
TS_CalibrationTypeDef calibration;

calibration.MinX = 300;
calibration.MaxX = 3700;
calibration.MinY = 300;
calibration.MaxY = 3700;

TS_SetCalibration(&hts, &calibration);
```

The crosshair UI and storing the values belong to the application, not the driver.

## API Reference

### Initialization Functions

- `TS_Init(hts, config)` - Initialize touchscreen controller on the bus and pins in `config`
- `TS_DeInit(hts)` - Deinitialize touchscreen controller
- `TS_Reset(hts)` - Reset touchscreen controller

### Touch Data Functions

- `TS_GetTouchData(hts, data)` - Get current touch data
- `TS_IsTouched(hts)` - Check if screen is currently touched
- `TS_GetSingleTouch(hts, touch)` - Get one mapped touch point
- `TS_GetTouchCount(hts, count)` - Number of active touches
- `TS_GetPressure(hts, pressure)` - Read touch pressure

### Configuration Functions

- `TS_Configure(hts, config)` - Apply a touchscreen configuration
- `TS_GetDefaultConfig()` - Configuration with interrupts on; bus, INT pin and display size left for the application
- `TS_RegisterCallbacks(hts, touch, release, gesture)` - Attach event callbacks
- `TS_SetActivityCallback(hts, cb)` - Attach the ISR-context callback run on every touch edge

### Calibration Functions

- `TS_SetCalibration(hts, calibration)` - Set the raw ADC bounds of the panel
- `TS_GetCalibration(hts, calibration)` - Get current calibration

### Interrupt Functions

- `TS_EnableInterrupt(hts, enable)` - Enable or disable touchscreen interrupts
- `TS_EXTI_Callback(hts)` - Note a touch edge; call from the application's `HAL_GPIO_EXTI_Callback`
- `TS_IrqPending(hts)` - Whether an edge is waiting to be serviced
- `TS_ServiceIRQ(hts)` - Clear the controller and run callbacks from thread context
- `TS_IRQHandler(hts)` - Handle touchscreen interrupt (called by `TS_ServiceIRQ`)

## Configuration Options

### Sample Time Settings
- `STMPE811_TSC_CFG_1_SAMPLE` - 1 sample per measurement
- `STMPE811_TSC_CFG_2_SAMPLE` - 2 samples averaged
- `STMPE811_TSC_CFG_4_SAMPLE` - 4 samples averaged
- `STMPE811_TSC_CFG_8_SAMPLE` - 8 samples averaged

### Touch Detection Delay
- `STMPE811_TSC_CFG_DELAY_10US` to `STMPE811_TSC_CFG_DELAY_50MS`

### Panel Driver Settling Time
- `STMPE811_TSC_CFG_SETTLE_10US` to `STMPE811_TSC_CFG_SETTLE_100MS`

## Gestures Supported

- `TS_GESTURE_TAP` - Single tap
- `TS_GESTURE_DOUBLE_TAP` - Double tap
- `TS_GESTURE_LONG_PRESS` - Long press (hold)
- `TS_GESTURE_SWIPE_UP` - Swipe up
- `TS_GESTURE_SWIPE_DOWN` - Swipe down
- `TS_GESTURE_SWIPE_LEFT` - Swipe left
- `TS_GESTURE_SWIPE_RIGHT` - Swipe right

## Examples

The `touchscreen_example.c` file provides comprehensive examples:

1. **Basic Touch Test** - Simple touch detection
2. **Touch Drawing** - Drawing application
3. **Calibration Test** - Interactive calibration
4. **Gesture Detection** - Gesture recognition demo
5. **Simple Menu** - Touch-based menu navigation
6. **Diagnostic Test** - Hardware diagnostics

## Troubleshooting

### Common Issues

1. **No Touch Detection**
   - Check I2C3 connections (PA8, PC9)
   - Verify device ID with `TS_ReadRegister(STMPE811_CHIP_ID, &id)`
   - Ensure proper power supply

2. **Inaccurate Touch Position**
   - Measure the raw corner values and apply them with `TS_SetCalibration()`
   - Check pressure threshold settings
   - Verify display coordinate mapping

3. **Missed Touches**
   - Enable interrupts for better responsiveness
   - Adjust pressure threshold
   - Check sampling configuration

4. **I2C Communication Errors**
   - Verify I2C3 clock configuration
   - Check pull-up resistors on I2C lines
   - Ensure correct device address (0x82)

### Debug Information

Enable debug output to monitor touchscreen operation:

```c
// In your configuration
#define TS_DEBUG_ENABLE 1

// This will print I2C transactions and touch events
```

## Hardware Specifications

- **Resolution**: 4096 x 4096 (12-bit ADC)
- **Display Mapping**: `displayWidth` x `displayHeight` from the configuration
- **Pressure Levels**: 4096 levels (12-bit)
- **Sample Rate**: Up to 80 Hz (configurable)
- **Power Supply**: 3.3V
- **Interface**: I2C (up to 400 kHz)

## Notes

- The STMPE811 is a resistive touchscreen controller, not capacitive
- Single-touch only (multi-touch not supported by hardware)
- Pressure sensitivity available for advanced applications
- Gesture recognition is implemented in software
- Calibration is recommended for accurate touch positioning
