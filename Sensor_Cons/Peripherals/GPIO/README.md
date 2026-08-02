# GPIO Driver for STM32F429

A small, portable GPIO driver. It is the layer every other peripheral in this
repository goes through to configure and drive pins, so porting to another STM32
family only requires replacing `gpio.c`.

## Files Overview

- **`gpio.h`** - The portable interface: configuration, pin/port state, interrupts
- **`gpio.c`** - STM32F4 implementation (RCC clock macros and the EXTI-to-IRQn map)
- **`GPIO_tutorial.ipynb`** - Walkthrough of the driver and the underlying registers

## Why this layer exists

`HAL_GPIO_Init()` will happily configure a pin whose port clock is still gated,
and the pin then does nothing. `GPIO_Driver_Pin_Init()` enables the port clock
first, so no caller has to remember `__HAL_RCC_GPIOx_CLK_ENABLE()`.

## API

### Configuration
- `GPIO_Driver_ClockEnable(GPIOx)` - Enable a port's peripheral clock
- `GPIO_Driver_Pin_Init(GPIOx, init)` - Configure pins (enables the clock first)
- `GPIO_Driver_Pin_DeInit(GPIOx, pin)` - Reset pins to their default state

### Pin state
- `GPIO_Driver_ReadPin(GPIOx, pin)` - Read one pin
- `GPIO_Driver_WritePin(GPIOx, pin, state)` - Drive one or more pins
- `GPIO_Driver_TogglePin(GPIOx, pin)` - Invert one or more pins

### Port state
- `GPIO_Driver_ReadPort(GPIOx)` - Read all 16 pins at once
- `GPIO_Driver_WritePort(GPIOx, value)` - Drive all 16 pins at once

### Interrupts
- `GPIO_Driver_EnableIRQ(pin, preempt, sub)` - Unmask the pin's EXTI line in the NVIC
- `GPIO_Driver_DisableIRQ(pin)` - Mask it again

## Usage Examples

### Output pin

```c
#include "gpio.h"

GPIO_InitTypeDef init = {0};
init.Pin   = GPIO_PIN_13;
init.Mode  = GPIO_MODE_OUTPUT_PP;
init.Pull  = GPIO_NOPULL;
init.Speed = GPIO_SPEED_FREQ_LOW;
GPIO_Driver_Pin_Init(GPIOG, &init);

GPIO_Driver_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_SET);
GPIO_Driver_TogglePin(GPIOG, GPIO_PIN_13);
```

### Input pin

```c
GPIO_InitTypeDef init = {0};
init.Pin  = GPIO_PIN_0;
init.Mode = GPIO_MODE_INPUT;
init.Pull = GPIO_PULLDOWN;
GPIO_Driver_Pin_Init(GPIOA, &init);

if (GPIO_Driver_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET) {
    /* pressed */
}
```

### Interrupt on a falling edge

The edge is chosen by the mode; `HAL_GPIO_Init` programs SYSCFG and EXTI for you.
The driver only adds the NVIC half:

```c
GPIO_InitTypeDef init = {0};
init.Pin  = GPIO_PIN_0;
init.Mode = GPIO_MODE_IT_FALLING;
init.Pull = GPIO_PULLUP;
GPIO_Driver_Pin_Init(GPIOA, &init);

GPIO_Driver_EnableIRQ(GPIO_PIN_0, 2, 0);
```

Handle it in `Core/Src/stm32f4xx_it.c`:

```c
void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void HAL_GPIO_EXTI_Callback(uint16_t pin)
{
    if (pin == GPIO_PIN_0) { /* ... */ }
}
```

### Parallel bus

```c
GPIO_Driver_WritePort(GPIOE, 0x00FF);
uint16_t value = GPIO_Driver_ReadPort(GPIOE);
```

## Notes

- `GPIO_Driver_EnableIRQ` takes a single-pin mask. EXTI lines 5-9 and 10-15 each
  share one NVIC channel, so the priority applies to every pin in that group.
- `GPIO_Driver_WritePort` writes the whole ODR, including pins owned by other
  drivers. Prefer `GPIO_Driver_WritePin` unless you are driving a parallel bus.
- Interrupt handlers live in `Core/Src/stm32f4xx_it.c`, never in this driver.
