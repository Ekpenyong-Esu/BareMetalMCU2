/**
  ******************************************************************************
  * @file    audio_i2s.c
  * @brief   I2S transport backend for the audio subsystem
  * @details Owns the I2S3 (SPI3) pin map, peripheral configuration and DMA
  *          stream selection. Nothing outside this file knows these details.
  ******************************************************************************
  */

#include "audio_i2s.h"
#include "gpio.h"

/* Private defines -----------------------------------------------------------*/

#define AUDIO_I2S_INSTANCE               SPI3
#define AUDIO_I2S_CLK_ENABLE()           __HAL_RCC_SPI3_CLK_ENABLE()
#define AUDIO_I2S_CLK_DISABLE()          __HAL_RCC_SPI3_CLK_DISABLE()

#define AUDIO_I2S_GPIO_PORT              GPIOC
#define AUDIO_I2S_WS_PIN                 GPIO_PIN_0
#define AUDIO_I2S_CK_PIN                 GPIO_PIN_10
#define AUDIO_I2S_SD_PIN                 GPIO_PIN_12

/**
 * @brief I2S3 (SPI3) transmit DMA
 * @note  SPI3_TX lives on DMA1, not DMA2. RM0090 offers DMA1 Stream 5 and
 *        Stream 7 on channel 0; Stream 5 is reserved for USART2_RX in this
 *        project, so Stream 7 is used.
 */
#define AUDIO_I2S_DMA_STREAM             DMA1_Stream7
#define AUDIO_I2S_DMA_CHANNEL            DMA_CHANNEL_0
#define AUDIO_I2S_DMA_IRQn               DMA1_Stream7_IRQn

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Route the I2S3 signals to their pins
 * @retval  None
 */
static void Audio_I2sInitPins(void)
{
    /* The GPIO driver enables the port clock. */
    GPIO_InitTypeDef gpioInit = {0};
    gpioInit.Pin = AUDIO_I2S_WS_PIN | AUDIO_I2S_CK_PIN | AUDIO_I2S_SD_PIN;
    gpioInit.Mode = GPIO_MODE_AF_PP;
    gpioInit.Pull = GPIO_NOPULL;
    gpioInit.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpioInit.Alternate = GPIO_AF6_SPI3;
    (void)GPIO_Driver_Pin_Init(AUDIO_I2S_GPIO_PORT, &gpioInit);
}

static AUDIO_StatusTypeDef Audio_I2sInit(AudioDevice_t* dev)
{
    AUDIO_I2S_CLK_ENABLE();
    Audio_I2sInitPins();

    dev->i2s.Instance = AUDIO_I2S_INSTANCE;
    dev->i2s.Init.Mode = I2S_MODE_MASTER_TX;
    dev->i2s.Init.Standard = I2S_STANDARD_PHILIPS;
    dev->i2s.Init.DataFormat = I2S_DATAFORMAT_16B;
    dev->i2s.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
    dev->i2s.Init.AudioFreq = Audio_SampleRateHz(dev->config.SampleRate);
    dev->i2s.Init.CPOL = I2S_CPOL_LOW;
    dev->i2s.Init.ClockSource = I2S_CLOCK_PLL;
    dev->i2s.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;

    if (HAL_I2S_Init(&dev->i2s) != HAL_OK) {
        return AUDIO_ERROR;
    }

    return AUDIO_OK;
}

static void Audio_I2sDeInit(AudioDevice_t* dev)
{
    (void)HAL_I2S_DeInit(&dev->i2s);
    AUDIO_I2S_CLK_DISABLE();
}

static void Audio_I2sLinkDma(AudioDevice_t* dev)
{
    __HAL_LINKDMA(&dev->i2s, hdmatx, dev->dma);
}

static AUDIO_StatusTypeDef Audio_I2sStart(AudioDevice_t* dev)
{
    /* HAL_I2S_Transmit_DMA counts 16-bit words, hence half the byte count. */
    if (HAL_I2S_Transmit_DMA(&dev->i2s, (uint16_t*)(void*)dev->output.Buffer,
                             (uint16_t)(dev->output.Size / 2U)) != HAL_OK) {
        return AUDIO_ERROR;
    }
    return AUDIO_OK;
}

static void Audio_I2sStop(AudioDevice_t* dev)
{
    (void)HAL_I2S_DMAStop(&dev->i2s);
}

static void Audio_I2sPause(AudioDevice_t* dev)
{
    (void)HAL_I2S_DMAPause(&dev->i2s);
}

static void Audio_I2sResume(AudioDevice_t* dev)
{
    (void)HAL_I2S_DMAResume(&dev->i2s);
}

/* Public data ---------------------------------------------------------------*/

const AudioIfOps_t AudioI2sOps = {
    .name       = "I2S",
    .dmaStream  = AUDIO_I2S_DMA_STREAM,
    .dmaChannel = AUDIO_I2S_DMA_CHANNEL,
    .dmaIrq     = AUDIO_I2S_DMA_IRQn,
    .init       = Audio_I2sInit,
    .deinit     = Audio_I2sDeInit,
    .linkDma    = Audio_I2sLinkDma,
    .start      = Audio_I2sStart,
    .stop       = Audio_I2sStop,
    .pause      = Audio_I2sPause,
    .resume     = Audio_I2sResume,
};
