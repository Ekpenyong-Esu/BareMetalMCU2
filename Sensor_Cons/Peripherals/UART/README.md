# UART Module Documentation

## Overview
This UART module provides a flexible, modular implementation for UART communication on the STM32F429I-DISC1 microcontroller. It supports multiple transfer modes (Blocking, Interrupt, and DMA) and includes a ring buffer implementation for efficient data handling.

## Architecture

### Core Components

There is no aggregator header: each module is included directly, so a file
declares exactly what it depends on. Application code normally needs only
`uart_core.h`.

1. **Types** (`uart_types.h`)
   - Shared vocabulary only: `UART_Status_t`, `UART_Mode_t`, `UART_Config_t`, `UART_Handle_t`, `UART_ModeOps_t`
   - No behaviour, so every module can include it without a dependency cycle

2. **Core** (`uart_core.h`, `uart.c`)
   - Opens and closes a link, routes transfers to the configured mode
   - Publishes the link currently being served via `UART_GetActiveHandle()`

4. **Events** (`uart_events.c`)
   - The HAL callbacks (`TxCplt`, `RxCplt`, `RxEvent`, `Error`)
   - All interrupt-context work lives here, and none of it branches on the mode
   - Exports no header: the callbacks are declared by the HAL, and the completion
     flags are fields on `UART_Handle_t`

5. **Configuration** (`uart_config.h`)
   - Tunables only: default settings, buffer sizes, timeouts, DMA stream selection

### Board Wiring (outside this driver)

The driver owns no board detail. Wiring lives in `Core/`, alongside every other
peripheral:

- `Core/Src/stm32f4xx_hal_msp.c` - `HAL_UART_MspInit()` / `MspDeInit()`: pins, clocks, DMA streams, NVIC
- `Core/Src/stm32f4xx_it.c` - `USART1_IRQHandler()`, `DMA2_Stream5/7_IRQHandler()`

Both resolve the link through `UART_GetActiveHandle()` rather than a global.

### Transfer Modes

Each mode exports exactly one symbol: a `UART_ModeOps_t` table. `UART_Init()` is
the only place that maps a `UART_Mode_t` onto one, so no other function switches
on the mode. A mode leaves an entry `NULL` when it has nothing to do — blocking
mode never runs in interrupt context, so it fills in only `init`, `transmit` and
`receive`.

6. **Blocking Mode** (`UART_BlockingOps`)
   - Synchronous transfers with every interrupt source disabled
   - Suitable for simple, low-throughput applications

7. **Interrupt Mode** (`UART_InterruptOps`)
   - Interrupt-driven transfers, completed by the callbacks in `uart_events.c`
   - Rebuilds the peripheral in `recoverFromError` after a line fault

8. **DMA Mode** (`UART_DmaOps`)
   - Buffer transfers offloaded to a DMA stream, minimal CPU overhead
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
        ├── uart_core.h        lifecycle + transfers
        ├── uart_blocking.h    UART_BlockingOps
        ├── uart_interrupt.h   UART_InterruptOps
        └── uart_dma.h         UART_DmaOps
```

`uart_ring_buffer.h` sits at the bottom and depends on nothing. Only `uart.c`
includes the three mode headers, so nothing else can reach past `uart_core.h`
into a specific mode.

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
    .mode = UART_MODE_TX_RX
};

// Initialize UART
UART_Handle_t uartHandle;
UART_Init(&uartHandle, &config);
```

### 2. Data Transfer

```c
// Transmit data
uint8_t data[] = "Hello World!";
UART_Transmit(&uartHandle, data, strlen((char*)data), 1000);

// Receive data
uint8_t rxBuffer[32];
UART_Receive(&uartHandle, rxBuffer, sizeof(rxBuffer), 1000);
```

### 3. Ring Buffer Usage

```c
// Initialize ring buffer
UART_RingBuffer_Init();

// Receive data using ring buffer
uint8_t data[32];
UART_RingBuffer_Receive(&uartHandle, data, sizeof(data));
```

## Configuration Options

Key configuration parameters in `uart_config.h`:

```c
/* Default UART configuration */
#define UART_DEFAULT_BAUDRATE     115200
#define UART_DEFAULT_WORDLENGTH   UART_WORDLENGTH_8B
#define UART_DEFAULT_STOPBITS     UART_STOPBITS_1
#define UART_DEFAULT_PARITY       UART_PARITY_NONE
#define UART_DEFAULT_MODE         UART_MODE_TX_RX

/* Buffer sizes */
#define RING_BUFFER_SIZE         512
#define RX_BUFFER_SIZE          512
#define TX_BUFFER_SIZE          512
```

## Integration with Main Application

The UART module is integrated into the main application through the following steps:

1. Include required headers in `main.c`:
   ```c
   #include "uart_core.h"
   ```

2. Initialize UART in the system initialization:
   ```c
   /* Initialize system components */
   SYS_Init();
   ```

3. Open a link and transfer using the mode it was configured with:
   ```c
   UART_Handle_t uart = { .huart = &huart1, .rxBuffer = rxBuf, .rxSize = sizeof(rxBuf) };
   UART_Config_t config = { .instance = USART1, .baudRate = UART_DEFAULT_BAUDRATE, .mode = UART_MODE_DMA };

   UART_Init(&uart, &config);
   UART_Transmit(&uart, data, len, UART_TIMEOUT);
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
