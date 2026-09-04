# LVGL Integration for STM32F429I-DISC1

LVGL (Light and Versatile Graphics Library) integration for the ILI9341 LCD
on STM32F429I-Discovery board using LTDC and SDRAM framebuffer.

## Features

- **LVGL v9** support with RGB565 color format
- **Double-buffered** SDRAM framebuffer (tear-free)
- **4-Screen GUI** - Home, Sensors, Settings, System Info
- **Touch support** ready (placeholder implementation)
- **Partial rendering** for memory efficiency

## Hardware Setup

```
STM32F429I-DISC1 LCD Stack:
┌─────────────────────────────────┐
│  LVGL (GUI rendering)           │
├─────────────────────────────────┤
│  lv_port_disp.c (flush to SDRAM)│
├─────────────────────────────────┤
│  LTDC (displays SDRAM content)  │
├─────────────────────────────────┤
│  ILI9341 (LCD controller)       │
├─────────────────────────────────┤
│  SDRAM (framebuffer storage)    │
└─────────────────────────────────┘
```

## Wiring and Bring-up

The drivers under `Peripherals/` do not know which pins or buses they sit on;
the application tells them. Every such choice for this board lives in
`gui_board.h` as a `GUI_BOARD_*` macro, and `gui_board.c` owns the bus
objects and driver handles built from them:

| Part | Wiring (STM32F429I-DISC1, all on-board) |
|------|------------------------------------------|
| ILI9341 command path | SPI5: PF7 SCK, PF8 MISO, PF9 MOSI; PC2 NCS, PD13 WRX; RST tied to NRST |
| ILI9341 pixel path | LTDC RGB lines (`HAL_LTDC_MspInit` in Core), PK3 backlight |
| STMPE811 touch | I2C3: PA8 SCL, PC9 SDA; INT on PA15 (EXTI15_10) |
| Framebuffer | FMC bank 2 SDRAM at 0xD0000000, RGB565 |

`GUI_Board_Init()` brings the hardware up in the order it needs: SDRAM, then
SPI5 + ILI9341 (RGB mode), then LTDC over the SDRAM framebuffer, then I2C3 +
STMPE811. `LVGL_App_Init()` calls it before registering the LVGL ports, so
`main()` only needs:

```c
#include "lvgl_app.h"

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    LVGL_App_Init();          // board bring-up + LVGL + GUI

    while(1) {
        LVGL_App_Tick();      // Process LVGL
        HAL_Delay(5);
    }
}
```

The LVGL ports and the low-power manager reach the handles through
`GUI_Board_Touch()`, `GUI_Board_Panel()`, `GUI_Board_Ltdc()` and
`GUI_Board_TouchBus()` rather than through driver singletons. `gui_board.c`
also defines `HAL_GPIO_EXTI_Callback`, forwarding the PA15 edge to
`TS_EXTI_Callback()` and clearing the PWR wakeup flag, so no other file in
the firmware may define that callback while this application is built.

## API Usage

### Update Display Values

```c
#include "lvgl_app.h"

// Update temperature gauge (0-100°C)
LVGL_App_UpdateTemperature(25);

// Update humidity bar (0-100%)
LVGL_App_UpdateHumidity(60);

// Add data point to sensor chart
LVGL_App_AddChartData(sensor_value);

// Update status message
LVGL_App_UpdateStatus(LV_SYMBOL_OK " System OK");
```

## Files

| File | Description |
|------|-------------|
| `gui_board.c/h` | Board wiring: pins, buses, driver handles, touch EXTI routing |
| `lv_port_disp.c/h` | Display driver - flushes to SDRAM |
| `lv_port_indev.c/h` | Touch input driver (placeholder) |
| `lvgl_app.c/h` | GUI application with 4 screens |
| `lv_conf.h` | LVGL configuration |

## Memory Layout (SDRAM)

```
0xD0000000  ┌─────────────────┐
            │  Framebuffer 0  │  153,600 bytes (240×320×2)
0xD0025800  ├─────────────────┤
            │  Framebuffer 1  │  153,600 bytes (240×320×2)
0xD004B000  ├─────────────────┤
            │  Available      │  ~7.7 MB remaining
            └─────────────────┘
```

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Black screen | Check SDRAM init, verify ILI9341 init before LTDC |
| Wrong colors | Check RGB565 format, may need BGR swap in lv_conf.h |
| Flickering | Double buffering issue, check swap_buffers() |
| Slow rendering | Increase DRAW_BUF_LINES in lv_port_disp.c |

## Dependencies

- SDRAM (Peripherals/SDRAM or FMC driver)
- ILI9341 (Peripherals/ILI9341)
- LTDC (Peripherals/LTDC)
- LVGL library (downloaded via CMake)
