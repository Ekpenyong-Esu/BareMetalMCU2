# UART Module Documentation

## Overview
This UART module provides a flexible, modular implementation for UART communication on the STM32F429I-DISC1 microcontroller. It supports multiple transfer modes (Blocking, Interrupt, and DMA) and includes a ring buffer implementation for efficient data handling.

## Architecture

### Core Components

There is no aggregator header: each module is included directly, so a file
declares exactly what it depends on. There is also no mode dispatch: each
transfer mode is an independent, directly-callable API, the same shape as
`tim_pwm.h`/`tim_ic.h` in the TIM driver.

1. **Types** (`uart_types.h`)
   - Shared vocabulary only: `UART_Status_t`, `UART_Mode_t`, `UART_Config_t`, `UART_Handle_t`
   - No behaviour, so every module can include it without a dependency cycle

2. **Shared utilities** (`uart.h`, `uart.c`)
   - `UART_DeInit()`, `UART_GetActiveHandle()`/`UART_SetActiveHandle()`
   - The UART equivalent of `tim_clock.h`: genuinely mode-agnostic helpers that
     each mode module depends on one-way. Holds no Init/Transmit/Receive of its
     own and never includes a mode header.

3. **Events** (`uart_events.c`)
   - The HAL callbacks (`TxCplt`, `RxCplt`, `RxEvent`, `Error`)
   - The one place that has to know which mode is active, since HAL exposes a
     single callback per event regardless of mode; it is a plain switch on
     `handle->config.mode`, not a vtable
   - Exports no header: the callbacks are declared by the HAL, and the completion
     flags are fields on `UART_Handle_t`

4. **Configuration** (`uart_config.h`)
   - Tunables only: default settings, buffer sizes, timeouts, DMA stream selection

### Board Wiring (outside this driver)

The driver owns no board detail. Wiring lives in `Core/`, alongside every other
peripheral:

- `Core/Src/stm32f4xx_hal_msp.c` - `HAL_UART_MspInit()` / `MspDeInit()`: pins, clocks, DMA streams, NVIC
- `Core/Src/stm32f4xx_it.c` - `USART1_IRQHandler()`, `DMA2_Stream5/7_IRQHandler()`

Both resolve the link through `UART_GetActiveHandle()` rather than a global.

### Transfer Modes

Each mode is a standalone module with its own `Init`/`Transmit`/`Receive`
functions, called directly by name — there is no shared `UART_Init()` and no
dispatch table to look one up in.

6. **Blocking Mode** (`uart_blocking.h`: `UART_Blocking_Init/Transmit/Receive`)
   - Synchronous transfers with every interrupt source disabled
   - Suitable for simple, low-throughput applications

7. **Interrupt Mode** (`uart_interrupt.h`: `UART_Interrupt_Init/Read/Write`)
   - Nothing waits. Reception is armed once by `Init()` and stays armed, so
     bytes arriving while the main loop is busy land in the ring instead of
     being lost. `Read()` drains the ring, `Write()` returns while the send
     is still going out and `IsTxDone()` says when the buffer is free again.
   - Also exports `UART_Interrupt_Rearm()`/`UART_Interrupt_Recover()`, called
     only by `uart_events.c` after a completed transfer or a line fault

8. **DMA Mode** (`uart_dma.h`: `UART_DMA_Init/Read/Write`)
   - Deliberately the same call set and the same non-blocking behaviour as
     interrupt mode. What differs is the engine: a DMA stream moves the bytes,
     so the CPU takes one interrupt per *transfer* instead of one per *byte*.
   - Buffers must live in DMA-reachable RAM. On the F429 that means ordinary
     SRAM — CCMRAM is not reachable by DMA2, and a stack buffer is gone before
     the stream is done with it, so the examples use `static`.
   - No cache maintenance: the Cortex-M4 here has no data cache for DMA to go
     stale against. That changes on M7 parts.
   - Also exports `UART_DMA_Rearm()`/`UART_DMA_Recover()`, called only by
     `uart_events.c` after a completed transfer or a line fault
   - Optimal for high-throughput applications

### Data Management

9. **Ring Buffer** (`uart_ring_buffer.h`, `uart_ring_buffer.c`)
   - A plain fixed-capacity byte queue with no UART dependency at all
   - One is embedded in every handle, so links cannot share received bytes
   - `RingBuffer_GetBytes()` is all-or-nothing, so a partial packet survives
     until a later call can complete it

## Dependencies

```
uart_ring_buffer.h      (no UART dependency)
  └── uart_types.h
        ├── uart.h              shared utilities (DeInit, active handle)
        ├── uart_blocking.h     UART_Blocking_Init/Transmit/Receive
        ├── uart_interrupt.h    UART_Interrupt_Init/Read/Write/IsTxDone/Rearm/Recover
        └── uart_dma.h          UART_DMA_Init/Read/Write/IsTxDone/Rearm/Recover
```

`uart_ring_buffer.h` sits at the bottom and depends on nothing. `uart.h` is a
pure utility module, the UART equivalent of `tim_clock.h`: each mode `.c` file
depends on it one-way (for `UART_DeInit`/`UART_SetActiveHandle`), and `uart.c`
never includes a mode header back. No file
switches on `UART_Mode_t` except `uart_events.c` (choosing how to react to a
HAL callback) — the driver itself has no dispatch table. `Applications/
uart_app/uart_interrupt_app.c` is the one example: it owns its own HAL handle
and calls `UART_Interrupt_Init`/`Read`/`Write` directly.


## Usage

### 1. Initialization

```c
// Configure UART
UART_Config_t config = {
    .instance = USART1,
    .baudRate = 115200,
    .wordLength = UART_WORDLENGTH_8B,
    .stopBits = UART_STOPBITS_1,
    .parity = UART_PARITY_NONE,
    .mode = UART_MODE_BLOCKING,   // just a record of which Init was used
};

// Open a link in whichever mode you want, by calling that mode's Init directly
UART_Handle_t uartHandle = { .huart = &huart1 };
UART_Blocking_Init(&uartHandle, &config);
```

### 2. Data Transfer

```c
// Transmit data
uint8_t data[] = "Hello World!";
UART_Blocking_Transmit(&uartHandle, data, strlen((char*)data), 1000);

// Receive data. `received` says how many bytes actually arrived; the read
// returns early once the line goes idle, so it is usually less than the
// buffer size. Bytes past it are untouched, so never call strlen() on rxBuffer.
uint8_t rxBuffer[32];
uint16_t received = 0;
UART_Blocking_Receive(&uartHandle, rxBuffer, sizeof(rxBuffer), &received, 1000);
```

Interrupt mode is not the same call set with a different prefix, because it
does not wait:

```c
static uint8_t rxLanding[64];   // the ISR receives into this; keep it alive
UART_Handle_t uartHandle = { .huart = &huart1 };
UART_Interrupt_Init(&uartHandle, &config, rxLanding, sizeof(rxLanding));

// In the super-loop. Both calls return immediately.
uint8_t rxBuffer[64];
uint16_t received = 0;
UART_Interrupt_Read(&uartHandle, rxBuffer, sizeof(rxBuffer), &received);
if (received > 0) { /* ... */ }

if (UART_Interrupt_IsTxDone(&uartHandle)) {          // last send has landed
    UART_Interrupt_Write(&uartHandle, txBuffer, txLength);  // txBuffer is the
}                                                    // driver's until IsTxDone
```

DMA mode is the same call set again with `UART_DMA_` in front, and the same
non-blocking behaviour — swapping one for the other is a find-and-replace. The
only extra rule is that its buffers must sit in DMA-reachable SRAM.

The driver has no dispatch table, so each mode is just a different set of
functions. `Applications/uart_app/` carries the example for whichever mode the
current branch is about — here `uart_dma_app.c`, which `main()` calls through
its single `UartDmaApp_Run()`. The interrupt-mode example lives on the
`feature/uart-interrupt` branch; the two are worth diffing, because they are
near-identical, which is exactly what the two modes are meant to show.

### 3. Ring Buffer Usage

```c
// The ring buffer is embedded in the handle; interrupt/DMA reception lands
// bytes there and they are drained with the container API.
uint8_t data[32];
if (RingBuffer_GetBytes(&uartHandle.rxRing, data, sizeof(data))) {
    // A full packet was copied; otherwise the buffer is left untouched.
}
```

## Configuration Options

Key configuration parameters in `uart_config.h`:

```c
/* Default UART configuration */
#define UART_DEFAULT_MODE         UART_MODE_TX_RX

/* Default timeout value in milliseconds */
#define UART_TIMEOUT             5000
```

`RING_BUFFER_SIZE` lives in `uart_ring_buffer.h` (the ring buffer is a
standalone container, so its capacity is not a UART tunable).

## Integration with Main Application

The UART module is integrated into the main application through the following steps:

1. Include required headers in `main.c`:
   ```c
   #include "uart.h"
   ```

2. Initialize UART in the system initialization:
   ```c
   /* Initialize system components */
   SYS_Init();
   ```

3. Open a link and transfer using the mode you called Init with:
   ```c
   UART_Handle_t uart = { .huart = &huart1, .rxBuffer = rxBuf, .rxSize = sizeof(rxBuf) };
   UART_Config_t config = { .instance = USART1, .baudRate = 115200, .mode = UART_MODE_DMA };

   UART_DMA_Init(&uart, &config);
   UART_DMA_Transmit(&uart, data, len, UART_TIMEOUT);
   ```

## Error Handling

The module includes comprehensive error handling:
- Status codes for all operations
- Timeout management
- Buffer overflow protection
- DMA error recovery

## Contributing

When modifying or extending this UART module:
1. Maintain the modular architecture
2. Follow the established error handling patterns
3. Update documentation for new features
4. Ensure backward compatibility

## License

This UART module is part of the STM32F429I-DISC1 bare metal implementation and follows the project's licensing terms.
