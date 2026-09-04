# Audio Driver for STM32F429

Playback driver for an I2S or SAI transport feeding an external codec
(Cirrus Logic CS43L22 over I2C). The driver never picks hardware: the
application owns an `AUDIO_Handle_t` and tells `AUDIO_Init()` which
instance, pins, DMA stream and control bus carry the audio.

## Features

- **Multiple Audio Interfaces**: SAI or I2S, selected per handle
- **Flexible Configuration**: Configurable sample rates, bit depths, and channel configurations
- **DMA Support**: Circular DMA playback from a buffer inside the handle
- **Codec Integration**: CS43L22 control over a caller-owned `I2C_Bus_t`
- **Volume Control**: Digital volume control with mute functionality
- **Statistics**: Sample counts, overflows and transfer errors per handle

## Files

- `audio_core.[ch]` - Handle lifecycle, registry, play/stop/pause/volume API
- `audio_types.h` - `AUDIO_ConfigTypeDef`, `AUDIO_Handle_t` and status codes
- `audio_i2s.c` / `audio_sai.c` - Transport backends; read instance and pins from the config
- `audio_dma.[ch]` - Transmit DMA stream setup from the config
- `audio_codec.[ch]` - CS43L22 register map, reset and volume translation
- `audio_buffer.[ch]` - Playback ring buffer bookkeeping
- `audio_events.c` - HAL callbacks and the DMA vector entry point

## Supported Audio Formats

- **Sample Rates**: 8kHz, 11.025kHz, 16kHz, 22.05kHz, 32kHz, 44.1kHz, 48kHz, 96kHz
- **Bit Depths**: 16-bit, 24-bit, 32-bit
- **Channels**: Mono and Stereo
- **Interfaces**: SAI1 Block A/B, or SPI2/SPI3 in I2S mode

## Wiring is supplied by the application

Nothing in this directory names a port, pin, DMA stream or I2C bus. The
config carries all of it; a pin with a NULL port is treated as not wired
(MCLK is then not driven, and the codec reset pulse is skipped).

```c
#include "audio_core.h"
#include "i2c.h"

static I2C_Bus_t      s_ctrlBus;   /* opened once by the application */
static AUDIO_Handle_t s_audio;     /* holds HAL handles, DMA memory, codec device */

void App_AudioInit(void)
{
    I2C_BusConfig_t busCfg = { .instance = I2C3, .sclPort = GPIOA, .sclPin = GPIO_PIN_8,
                               .sdaPort = GPIOC, .sdaPin = GPIO_PIN_9 };
    I2C_BusInit(&s_ctrlBus, &busCfg);

    AUDIO_ConfigTypeDef cfg = {
        .Interface  = AUDIO_INTERFACE_SAI,
        .SampleRate = AUDIO_FREQ_44K,
        .BitDepth   = AUDIO_FORMAT_16BIT,
        .Channels   = AUDIO_CHANNEL_STEREO,
        .BufferSize = AUDIO_BUFFER_SIZE_DEFAULT,
        .EnableDMA  = true,

        .saiBlock   = SAI1_Block_A,
        .mclkPin    = { GPIOE, GPIO_PIN_2 },
        .sdPin      = { GPIOE, GPIO_PIN_4 },
        .wsPin      = { GPIOE, GPIO_PIN_5 },   /* FS */
        .ckPin      = { GPIOE, GPIO_PIN_6 },   /* SCK */
        .alternate  = 0,                       /* derive from the instance */
        .dmaStream  = DMA2_Stream3,
        .dmaChannel = DMA_CHANNEL_0,

        .codecBus      = &s_ctrlBus,           /* NULL = transport only */
        .codecAddress  = 0,                    /* 0 = CS43L22 default 0x94 */
        .codecResetPin = { GPIOD, GPIO_PIN_4 },
    };
    AUDIO_Init(&s_audio, &cfg);
}
```

For I2S set `.Interface = AUDIO_INTERFACE_I2S`, `.i2sInstance = SPI3` (or
SPI2) and the WS/CK/SD pins; `alternate` resolves to AF5 for SPI2 and AF6
for SPI3 unless overridden.

The DMA stream named in the config must have its vector routed to
`AUDIO_IRQHandler()` in `stm32f4xx_it.c`. The handler services every
registered handle, so one call serves any stream.

### Audio Playback

```c
uint8_t audioData[4096];

AUDIO_WriteBuffer(&s_audio, audioData, sizeof(audioData));
AUDIO_Play(&s_audio);
/* ... playback runs in the background via DMA ... */
AUDIO_Stop(&s_audio);
```

### Volume Control

```c
AUDIO_SetVolume(&s_audio, 50);
AUDIO_SetMute(&s_audio, true);
AUDIO_SetMute(&s_audio, false);
```

Volume and mute return `AUDIO_NOT_READY` on a handle opened without a
codec bus.

## Buffer Sizes

- Default and maximum buffer size: `AUDIO_BUFFER_SIZE_DEFAULT` (4096 bytes),
  reserved inside each `AUDIO_Handle_t`
- Place the handle in DMA-reachable RAM (not CCM)

## Error Handling

- `AUDIO_OK` - Operation successful
- `AUDIO_ERROR` - General error
- `AUDIO_BUSY` - Handle already registered or no registry slot free
- `AUDIO_TIMEOUT` - Operation timeout
- `AUDIO_INVALID_PARAM` - Missing instance, pin or stream in the config
- `AUDIO_NOT_READY` - Handle not initialised, or no codec on this handle
- `AUDIO_OVERFLOW` - Buffer overflow
- `AUDIO_UNDERFLOW` - Buffer underflow

## Troubleshooting

1. **No Audio Output**: Check codec power, reset line and I2C bus
2. **Distorted Audio**: Verify sample rate and bit depth settings
3. **Buffer Overflows**: Increase buffer size or reduce sample rate
4. **DMA Errors**: Check that the stream/channel pair serves the chosen instance
   (RM0090 table 42/43) and that its vector calls `AUDIO_IRQHandler()`

Use `AUDIO_GetStatistics()` to monitor sample count, overflows and
synchronization errors.

## Dependencies

- STM32F4xx HAL library (I2S, SAI, DMA, GPIO)
- `Peripherals/I2C` bus driver for codec control
- `Peripherals/DMA` for stream-to-IRQ lookup
- `Peripherals/GPIO` for pin setup
