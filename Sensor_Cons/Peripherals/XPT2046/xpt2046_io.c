/**
 * @file xpt2046_io.c
 * @brief Internal SPI/GPIO transport for the XPT2046 controller
 */

#include "xpt2046_io.h"
#include "spi.h"
#include "gpio.h"

/* Differential-mode control bytes: S=1, MODE=0 (12-bit), SER/DFR=0, PD=00 so the
   reference and drivers power down between conversions and PENIRQ stays alive.
   Channel select A2:A0 per the ADS7846/XPT2046 conversion table. */
#define XPT2046_CMD_READ_X            0xD0U   /**< A2:A0 = 101, X position */
#define XPT2046_CMD_READ_Y            0x90U   /**< A2:A0 = 001, Y position */
#define XPT2046_CMD_READ_Z1           0xB0U   /**< A2:A0 = 011, Z1 */
#define XPT2046_CMD_READ_Z2           0xC0U   /**< A2:A0 = 100, Z2 */

#define XPT2046_CS_SETUP_US           10U     /**< Chip select setup time */
#define XPT2046_FRAME_BYTES           3U      /**< Command byte plus two result bytes */
#define XPT2046_CHANNEL_COUNT         4U
#define XPT2046_SEQUENCE_BYTES        (XPT2046_FRAME_BYTES * XPT2046_CHANNEL_COUNT)

#define XPT2046_CYCLES_PER_LOOP       4U      /**< Approximate cost of one delay iteration */
#define XPT2046_HZ_PER_MHZ            1000000U

/* This controller's slot on the shared bus. */
static SPI_Device_t s_device;

void XPT2046_IO_DelayUs(uint32_t microseconds)
{
    volatile uint32_t count = (microseconds * (SystemCoreClock / XPT2046_HZ_PER_MHZ)) /
                              XPT2046_CYCLES_PER_LOOP;

    while (count-- > 0U) {
        __NOP();
    }
}

XPT2046_StatusTypeDef XPT2046_IO_ConfigurePins(const XPT2046_Config_t *config)
{
    GPIO_InitTypeDef init = {0};

    if (config == NULL || config->cs_port == NULL || config->irq_port == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    init.Pin = config->cs_pin;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_Driver_Pin_Init(config->cs_port, &init);
    HAL_GPIO_WritePin(config->cs_port, config->cs_pin, GPIO_PIN_SET);

    init.Pin = config->irq_pin;
    init.Mode = GPIO_MODE_INPUT;
    init.Pull = GPIO_PULLUP;
    init.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_Driver_Pin_Init(config->irq_port, &init);

    const SPI_ConfigTypeDef spiConfig = SPI_ConfigDefault();
    if (SPI_DeviceInit(&s_device, &spiConfig) != SPI_OK) {
        return XPT2046_ERROR;
    }

    return XPT2046_OK;
}

bool XPT2046_IO_PenDown(const XPT2046_Config_t *config)
{
    if (config == NULL || config->irq_port == NULL) {
        return false;
    }

    return (HAL_GPIO_ReadPin(config->irq_port, config->irq_pin) == GPIO_PIN_RESET);
}

static uint16_t XPT2046_IO_Decode(const uint8_t *frame)
{
    /* The converter emits a busy bit, then D11..D0, then three trailing zeros. */
    return (uint16_t)(((uint16_t)(frame[1] & 0x7FU) << 5) | (uint16_t)(frame[2] >> 3));
}

XPT2046_StatusTypeDef XPT2046_IO_ReadSample(const XPT2046_Config_t *config,
                                            XPT2046_RawSample_t *sample)
{
    uint8_t tx[XPT2046_SEQUENCE_BYTES] = {
        XPT2046_CMD_READ_X,  0x00U, 0x00U,
        XPT2046_CMD_READ_Y,  0x00U, 0x00U,
        XPT2046_CMD_READ_Z1, 0x00U, 0x00U,
        XPT2046_CMD_READ_Z2, 0x00U, 0x00U
    };
    uint8_t rx[XPT2046_SEQUENCE_BYTES] = {0};
    SPI_StatusTypeDef spiStatus;

    if (config == NULL || sample == NULL || config->cs_port == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    /* One chip select window for all four channels: releasing CS between them
       would restart the acquisition and let the panel settle differently. */
    HAL_GPIO_WritePin(config->cs_port, config->cs_pin, GPIO_PIN_RESET);
    XPT2046_IO_DelayUs(XPT2046_CS_SETUP_US);

    spiStatus = SPI_TransmitReceive(&s_device, tx, rx, XPT2046_SEQUENCE_BYTES, SPI_TIMEOUT_SHORT);

    HAL_GPIO_WritePin(config->cs_port, config->cs_pin, GPIO_PIN_SET);

    if (spiStatus != SPI_OK) {
        return (spiStatus == SPI_TIMEOUT) ? XPT2046_TIMEOUT : XPT2046_ERROR;
    }

    sample->x = XPT2046_IO_Decode(&rx[0]);
    sample->y = XPT2046_IO_Decode(&rx[XPT2046_FRAME_BYTES]);
    sample->z1 = XPT2046_IO_Decode(&rx[XPT2046_FRAME_BYTES * 2U]);
    sample->z2 = XPT2046_IO_Decode(&rx[XPT2046_FRAME_BYTES * 3U]);

    return XPT2046_OK;
}
