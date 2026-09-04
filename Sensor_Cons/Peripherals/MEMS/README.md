# MEMS Sensor Driver for STM32F429 Discovery Board

This directory contains a comprehensive MEMS (Micro-Electro-Mechanical Systems) sensor driver implementation for the STM32F429 Discovery board, specifically designed for the L3GD20 3-axis digital gyroscope.

**Note:** The older `Peripherals/GYRO` driver was removed — use this `MEMS` driver for both the on-board and external gyroscopes; the chip-select pin is part of `MEMS_Config_t`.

## Overview

The MEMS driver provides a complete interface for controlling and reading data from the L3GD20 gyroscope sensor over SPI. The driver includes features for initialization, calibration, data reading, power management, and interrupt handling. It does not know the board: the application opens the SPI bus and tells `MEMS_Init()` which chip-select and interrupt pins the part is wired to.

## Hardware Configuration

### L3GD20 Gyroscope Sensor
- **Connection**: 4-wire SPI, mode 0, up to 10 MHz (the driver fixes these)
- **Full Scale Ranges**: ±250, ±500, ±2000 dps (degrees per second)
- **Output Data Rates**: 95Hz, 190Hz, 380Hz, 760Hz
- **Resolution**: 16-bit
- **Operating Voltage**: 2.16V to 3.6V

### Pin Configuration
SCK/MISO/MOSI belong to the `SPI_Bus_t` the application opens; CS, INT1 and
INT2 are given to `MEMS_Init()` in `MEMS_Config_t`. Leave an INT port NULL if
that line is not connected. On the STM32F429 Discovery the on-board part sits
on SPI5 (PF7/PF8/PF9) with CS on PC1, INT1 on PA1 and INT2 on PA2.

## Files Structure

```
MEMS/
├── mems_l3gd20.h       # L3GD20 register map and bit definitions
├── mems_types.h        # Status, enums, config structs, handle
├── mems_hw.h/.c        # Chip select and INT pins of one part
├── mems_io.h/.c        # Register read/write/update over SPI
├── mems_convert.h/.c   # Raw sample to degrees per second
├── mems_gyro.h/.c      # Gyro configuration and sample acquisition
├── mems_interrupt.h/.c # CTRL_REG3 interrupt routing
├── mems_calibration.h/.c # Zero-rate offset calibration
├── mems_diag.h/.c      # Device info, status, temperature, self-test
├── mems_core.h/.c      # Init / DeInit / Reset / default config
└── README.md           # This documentation file
```

## Features

### Core Features
- ✅ Device initialization and configuration
- ✅ Gyroscope data reading (raw and engineering units)
- ✅ Temperature sensor reading
- ✅ Automatic calibration with offset compensation
- ✅ Power management (normal/power-down modes)
- ✅ Interrupt configuration and handling
- ✅ Self-test functionality
- ✅ Multiple full-scale ranges and data rates
- ✅ Error handling and status reporting

### Advanced Features
- ✅ Motion detection algorithms
- ✅ Statistical data analysis
- ✅ Performance testing utilities
- ✅ Comprehensive example implementations
- ✅ Device information and status monitoring

## API Reference

### Initialization Functions
```c
MEMS_StatusTypeDef MEMS_Init(MEMS_HandleTypeDef *hmems, const MEMS_Config_t *config);
MEMS_StatusTypeDef MEMS_DeInit(MEMS_HandleTypeDef *hmems);
```

`config` names the open bus and the CS/INT pins. The driver registers its own
`SPI_Device_t` with the settings the L3GD20 needs, so the bus is reprogrammed
to them whenever the gyro is selected and another chip sharing the wires
cannot leave it misconfigured.

### Configuration Functions
```c
MEMS_StatusTypeDef MEMS_GyroConfig(MEMS_HandleTypeDef *hmems, MEMS_GyroConfigTypeDef *config);
MEMS_StatusTypeDef MEMS_ConfigureInterrupt(MEMS_HandleTypeDef *hmems, MEMS_InterruptConfigTypeDef *config);
MEMS_StatusTypeDef MEMS_SetPowerMode(MEMS_HandleTypeDef *hmems, bool power_down);
```

### Data Reading Functions
```c
MEMS_StatusTypeDef MEMS_GyroReadRaw(MEMS_HandleTypeDef *hmems, MEMS_AxesRawTypeDef *axes);
MEMS_StatusTypeDef MEMS_GyroRead(MEMS_HandleTypeDef *hmems, MEMS_AxesTypeDef *axes);
MEMS_StatusTypeDef MEMS_ReadTemperature(MEMS_HandleTypeDef *hmems, float *temperature);
```

### Calibration Functions
```c
MEMS_StatusTypeDef MEMS_CalibrateGyroscope(MEMS_HandleTypeDef *hmems, uint16_t samples);
```

### Utility Functions
```c
MEMS_StatusTypeDef MEMS_GetDeviceInfo(MEMS_HandleTypeDef *hmems, MEMS_DeviceInfoTypeDef *info);
MEMS_StatusTypeDef MEMS_SelfTest(MEMS_HandleTypeDef *hmems, bool *result);
float MEMS_ConvertToDPS(int16_t raw_data, MEMS_GyroFullScaleTypeDef full_scale);
```

## Usage Examples

### Basic Initialization and Reading

```c
#include "spi_core.h"
#include "mems_core.h"
#include "mems_gyro.h"

// The application owns the bus and decides which pins carry it.
SPI_BusConfig_t busConfig = {
    .instance = SPI5,
    .sckPort = GPIOF,  .sckPin = GPIO_PIN_7,
    .misoPort = GPIOF, .misoPin = GPIO_PIN_8,
    .mosiPort = GPIOF, .mosiPin = GPIO_PIN_9,
};
SPI_Bus_t bus;
SPI_BusInit(&bus, &busConfig);

// On-board part; an external one only differs in these pins.
MEMS_Config_t memsConfig = {
    .Bus = &bus,
    .CS_Port = GPIOC,   .CS_Pin = GPIO_PIN_1,
    .INT1_Port = GPIOA, .INT1_Pin = GPIO_PIN_1,
    .INT2_Port = GPIOA, .INT2_Pin = GPIO_PIN_2,   /* or NULL port if unused */
};

MEMS_HandleTypeDef hmems;
MEMS_AxesTypeDef gyro_data;

if (MEMS_Init(&hmems, &memsConfig) == MEMS_OK) {
    // Read gyroscope data
    if (MEMS_GyroRead(&hmems, &gyro_data) == MEMS_OK) {
        printf("Gyro X: %.2f dps\n", gyro_data.X);
        printf("Gyro Y: %.2f dps\n", gyro_data.Y);
        printf("Gyro Z: %.2f dps\n", gyro_data.Z);
    }
}
```

### Configuration Example
```c
MEMS_GyroConfigTypeDef config = {
    .OutputDataRate = MEMS_GYRO_ODR_190Hz,
    .FullScale = MEMS_GYRO_FULLSCALE_500,
    .Bandwidth = MEMS_GYRO_BANDWIDTH_2,
    .XAxisEnable = true,
    .YAxisEnable = true,
    .ZAxisEnable = true,
    .PowerDownMode = false
};

MEMS_GyroConfig(&hmems, &config);
```

### Calibration Example
```c
// Perform calibration with 100 samples
printf("Keep device stationary for calibration...\n");
if (MEMS_CalibrateGyroscope(&hmems, 100) == MEMS_OK) {
    printf("Calibration completed successfully\n");
    printf("X offset: %.3f dps\n", hmems.CalibrationOffset.X);
    printf("Y offset: %.3f dps\n", hmems.CalibrationOffset.Y);
    printf("Z offset: %.3f dps\n", hmems.CalibrationOffset.Z);
}
```

## Integration Guide

### 1. Hardware Setup
Ensure that the STM32F429 Discovery board is properly connected and powered. The L3GD20 sensor is already mounted on the board and connected to the SPI5 interface.

### 2. Include Headers
```c
#include "mems_core.h"        // Init / DeInit / Reset / default config
#include "mems_gyro.h"        // Configuration and sample acquisition
#include "mems_calibration.h" // Zero-rate offset calibration
#include "mems_diag.h"        // Device info, status, temperature, self-test
#include "mems_interrupt.h"   // Interrupt routing
```

### 3. Open the bus
The SPI peripheral and its SCK/MISO/MOSI pins are owned by the application
through `Peripherals/SPI`; open it once with `SPI_BusInit()` and hand the
`SPI_Bus_t` to every device driver sharing it. The L3GD20's own bus settings
(mode 0, PCLK/16) are fixed inside this driver.

### 4. Application Integration
```c
int main(void) {
    HAL_Init();
    SystemClock_Config();

    SPI_Bus_t bus;
    SPI_BusInit(&bus, &busConfig);           /* see Basic Initialization */

    MEMS_HandleTypeDef hmems;

    // Initialize MEMS sensor
    if (MEMS_Init(&hmems, &memsConfig) == MEMS_OK) {
        // Run basic example
        MEMS_ExampleResultTypeDef result = MEMS_Example_Basic(&hmems);
        MEMS_Example_PrintResult(&result);
        
        // Your application code here
        while (1) {
            MEMS_AxesTypeDef gyro;
            if (MEMS_GyroRead(&hmems, &gyro) == MEMS_OK) {
                // Process gyroscope data
            }
            HAL_Delay(10);
        }
    }
    
    return 0;
}
```

## Error Handling

The driver uses comprehensive error codes for robust error handling:

```c
typedef enum {
    MEMS_OK = 0,                // Operation completed successfully
    MEMS_ERROR,                 // General error occurred
    MEMS_BUSY,                  // MEMS device is busy
    MEMS_TIMEOUT,               // Operation timed out
    MEMS_INVALID_PARAM,         // Invalid parameter provided
    MEMS_NOT_INITIALIZED,       // Device not initialized
    MEMS_COMMUNICATION_ERROR,   // SPI communication error
    MEMS_DEVICE_NOT_FOUND      // Device not detected
} MEMS_StatusTypeDef;
```

### Error Handling Example
```c
MEMS_StatusTypeDef status = MEMS_GyroRead(&hmems, &gyro_data);
switch (status) {
    case MEMS_OK:
        // Process data
        break;
    case MEMS_NOT_INITIALIZED:
        printf("ERROR: MEMS not initialized\n");
        break;
    case MEMS_COMMUNICATION_ERROR:
        printf("ERROR: SPI communication failed\n");
        break;
    default:
        printf("ERROR: Unknown error (%d)\n", status);
        break;
}
```

## Performance Considerations

### Timing Requirements
- **Initialization Time**: ~10ms
- **Reading Time**: ~1ms per axis set
- **Calibration Time**: ~2-5 seconds (depending on sample count)
- **Maximum Data Rate**: Up to 760Hz (limited by SPI speed)

### Memory Usage
- **Driver Structure**: ~100 bytes
- **Stack Usage**: ~50 bytes per function call
- **No dynamic memory allocation**

### Power Consumption
- **Normal Mode**: ~6.1mA
- **Power-Down Mode**: ~5µA
- Use power-down mode when not actively reading data

## Troubleshooting

### Common Issues

1. **Device Not Found (MEMS_DEVICE_NOT_FOUND)**
   - Check SPI connections
   - Verify power supply
   - Ensure correct pin configuration

2. **Communication Errors**
   - Verify SPI clock frequency (max 10MHz)
   - Check CS pin control
   - Ensure proper SPI mode configuration

3. **Noisy Data**
   - Perform calibration
   - Check for mechanical vibrations
   - Use appropriate full-scale range

4. **Calibration Issues**
   - Ensure device is stationary during calibration
   - Use sufficient number of samples (>100)
   - Allow settling time before calibration

### Debug Tips
```c
// Enable debug output
#define MEMS_DEBUG_ENABLE

// Check device presence
MEMS_DeviceInfoTypeDef info;
if (MEMS_GetDeviceInfo(&hmems, &info) == MEMS_OK) {
    printf("Device: %s, WHO_AM_I: 0x%02X\n", info.DeviceName, info.WhoAmI);
}

// Perform self-test
bool test_result;
if (MEMS_SelfTest(&hmems, &test_result) == MEMS_OK) {
    printf("Self-test: %s\n", test_result ? "PASS" : "FAIL");
}
```

## Dependencies

### HAL Libraries
- `stm32f4xx_hal.h`
- `stm32f4xx_hal_spi.h`
- `stm32f4xx_hal_gpio.h`

### Standard Libraries
- `<stdint.h>`
- `<stdbool.h>`
- `<string.h>`
- `<math.h>` (for magnitude calculations)

## Examples

The `mems_example.c` file provides comprehensive examples:

1. **Basic Example**: Initialization and basic reading
2. **Continuous Reading**: Data collection over time
3. **Calibration**: Offset compensation
4. **Temperature Monitoring**: Temperature sensor usage
5. **Motion Detection**: Movement detection algorithms
6. **Self-Test**: Built-in diagnostic
7. **Power Management**: Power mode control
8. **Performance Test**: Speed and reliability testing

Run examples with:
```c
MEMS_ExampleResultTypeDef result = MEMS_Example_Basic(&hmems);
MEMS_Example_PrintResult(&result);
```

## License

This MEMS driver is provided as part of the STM32F429 Discovery board peripheral driver collection.

## Version History

- **v1.0** (2025-09-04): Initial release
  - L3GD20 gyroscope support
  - Complete API implementation
  - Comprehensive examples
  - Full documentation

## Support

For technical support or questions about this driver, please refer to:
- STM32F429 Reference Manual
- L3GD20 Datasheet
- STM32F429 Discovery Board User Manual
