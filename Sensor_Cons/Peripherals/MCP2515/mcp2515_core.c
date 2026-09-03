/**
  ******************************************************************************
  * @file    mcp2515_core.c
  * @brief   Lifecycle, mode and bit timing for the MCP2515
  ******************************************************************************
  */

#include "mcp2515_core.h"
#include "mcp2515_io.h"
#include "mcp2515_regs.h"
#include "gpio.h"
#include "log.h"

/* Private constants ---------------------------------------------------------*/

#define MCP2515_MODE_TIMEOUT_MS 10U
#define MCP2515_BRP_MAX         64U     /*!< BRP field is 6 bits, so 1..64 divisions */

/* Segment limits from the datasheet, in time quanta */
#define MCP2515_PROP_MAX        8U
#define MCP2515_PS1_MAX         8U
#define MCP2515_PS2_MIN         2U

/** Preferred bit lengths, longest first: more quanta place the sample point better. */
static const uint8_t s_tqPerBitOptions[] = { 16U, 15U, 14U, 12U, 10U, 8U };
#define MCP2515_TQ_OPTION_COUNT (sizeof(s_tqPerBitOptions) / sizeof(s_tqPerBitOptions[0]))

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Configure the chip-select pin and idle it high.
 */
static MCP2515_Status_t MCP2515_InitCsPin(const MCP2515_Config_t *config)
{
    GPIO_InitTypeDef gpio = {0};

    gpio.Pin = config->cs_pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    if (GPIO_Driver_Pin_Init(config->cs_port, &gpio) != HAL_OK) {
        return MCP2515_ERROR;
    }

    HAL_GPIO_WritePin(config->cs_port, config->cs_pin, GPIO_PIN_SET);

    return MCP2515_OK;
}

/**
 * @brief Bus settings the part requires, which is why they are not configurable.
 * @note  Mode 0,0 and at most 10 MHz; PCLK2/16 keeps the clock inside that.
 */
static SPI_ConfigTypeDef MCP2515_BusConfig(void)
{
    SPI_ConfigTypeDef config = SPI_ConfigDefault();

    config.CLKPolarity = SPI_POLARITY_LOW;
    config.CLKPhase = SPI_PHASE_1EDGE;
    config.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;

    return config;
}

/* Exported functions --------------------------------------------------------*/

MCP2515_Status_t MCP2515_SetBitRate(MCP2515_Handle_t *hmcp, uint32_t oscillator_hz,
                                    uint32_t baud_rate)
{
    uint32_t divisor = 0U;
    uint32_t tqPerBit = 0U;
    uint32_t brp = 0U;
    uint8_t prop = 0U;
    uint8_t ps1 = 0U;
    uint8_t ps2 = 0U;
    uint8_t cnf1 = 0U;
    uint8_t cnf2 = 0U;
    uint8_t cnf3 = 0U;

    if (hmcp == NULL || oscillator_hz == 0U || baud_rate == 0U) {
        return MCP2515_INVALID_PARAM;
    }

    /* One time quantum is 2*(BRP+1)/Fosc, so a bit of N quanta needs
       Fosc / (2 * bitrate) == N * (BRP+1). Derive it rather than keeping a
       table, which silently produces the wrong rate on a different crystal. */
    if ((oscillator_hz % (2U * baud_rate)) != 0U) {
        return MCP2515_BITRATE_UNSUPPORTED;
    }
    divisor = oscillator_hz / (2U * baud_rate);

    for (size_t i = 0; i < MCP2515_TQ_OPTION_COUNT; i++) {
        uint32_t candidate = s_tqPerBitOptions[i];

        if ((divisor % candidate) == 0U && (divisor / candidate) <= MCP2515_BRP_MAX) {
            tqPerBit = candidate;
            brp = (divisor / candidate) - 1U;
            break;
        }
    }

    if (tqPerBit == 0U) {
        return MCP2515_BITRATE_UNSUPPORTED;
    }

    /* Spend a quarter of the bit after the sample point, then give PS1 as much
       of the rest as it can hold. That lands the sample at 75-80%, inside the
       window CiA recommends. */
    ps2 = (uint8_t)(tqPerBit / 4U);
    if (ps2 < MCP2515_PS2_MIN) {
        ps2 = MCP2515_PS2_MIN;
    }

    ps1 = (uint8_t)(tqPerBit - 1U - ps2 - 1U); /* the trailing 1 is the smallest PropSeg */
    if (ps1 > MCP2515_PS1_MAX) {
        ps1 = MCP2515_PS1_MAX;
    }

    prop = (uint8_t)(tqPerBit - 1U - ps2 - ps1);
    if (prop == 0U || prop > MCP2515_PROP_MAX) {
        return MCP2515_BITRATE_UNSUPPORTED;
    }

    /* Register fields are the segment lengths minus one; SJW stays at 1 tq. */
    cnf1 = (uint8_t)brp;
    cnf2 = (uint8_t)(0x80U                       /* BTLMODE: PS2 comes from CNF3 */
                     | ((uint8_t)(ps1 - 1U) << 3)
                     | (uint8_t)(prop - 1U));
    cnf3 = (uint8_t)(ps2 - 1U);

    if (MCP2515_WriteRegister(hmcp, MCP2515_REG_CNF1, cnf1) != MCP2515_OK ||
        MCP2515_WriteRegister(hmcp, MCP2515_REG_CNF2, cnf2) != MCP2515_OK ||
        MCP2515_WriteRegister(hmcp, MCP2515_REG_CNF3, cnf3) != MCP2515_OK) {
        return MCP2515_ERROR;
    }

    return MCP2515_OK;
}

MCP2515_Status_t MCP2515_SetMode(MCP2515_Handle_t *hmcp, MCP2515_Mode_t mode)
{
    uint32_t start;

    MCP2515_Status_t status = MCP2515_ModifyRegister(hmcp, MCP2515_REG_CANCTRL,
                                                     MCP2515_CANCTRL_REQOP_MASK,
                                                     (uint8_t)mode);
    if (status != MCP2515_OK) {
        return status;
    }

    /* A mode request is not instant: the device finishes the frame in flight
       first, so read back rather than assuming it took. */
    start = HAL_GetTick();
    do {
        uint8_t canstat = 0U;

        status = MCP2515_ReadRegister(hmcp, MCP2515_REG_CANSTAT, &canstat);
        if (status != MCP2515_OK) {
            return status;
        }

        if ((canstat & MCP2515_CANSTAT_OPMOD_MASK) == (uint8_t)mode) {
            return MCP2515_OK;
        }
    } while ((HAL_GetTick() - start) < MCP2515_MODE_TIMEOUT_MS);

    return MCP2515_TIMEOUT;
}

MCP2515_Status_t MCP2515_Init(MCP2515_Handle_t *hmcp, const MCP2515_Config_t *config)
{
    SPI_ConfigTypeDef busConfig;
    MCP2515_Status_t status;

    if (hmcp == NULL || config == NULL || config->cs_port == NULL || config->cs_pin == 0U) {
        return MCP2515_INVALID_PARAM;
    }

    log_debug("MCP2515: Initializing");

    hmcp->cs_port = config->cs_port;
    hmcp->cs_pin = config->cs_pin;
    hmcp->initialized = false;

    if (MCP2515_InitCsPin(config) != MCP2515_OK) {
        return MCP2515_ERROR;
    }

    busConfig = MCP2515_BusConfig();
    if (SPI_DeviceInit(&hmcp->device, &busConfig) != SPI_OK) {
        return MCP2515_ERROR;
    }

    status = MCP2515_ResetDevice(hmcp);
    if (status != MCP2515_OK) {
        return status;
    }

    /* Reset leaves the device in configuration mode, the only mode in which the
       timing registers accept writes. */
    status = MCP2515_SetMode(hmcp, MCP2515_MODE_CONFIG);
    if (status != MCP2515_OK) {
        return status;
    }

    status = MCP2515_SetBitRate(hmcp, config->oscillator_hz, config->baud_rate);
    if (status != MCP2515_OK) {
        return status;
    }

    /* Accept every frame: filters are a separate decision, and silently
       dropping traffic is a worse default than delivering too much. */
    status = MCP2515_WriteRegister(hmcp, MCP2515_REG_RXB0CTRL,
                                   MCP2515_RXBCTRL_RXM_ANY | MCP2515_RXB0CTRL_BUKT);
    if (status != MCP2515_OK) {
        return status;
    }

    status = MCP2515_WriteRegister(hmcp, MCP2515_REG_RXB1CTRL, MCP2515_RXBCTRL_RXM_ANY);
    if (status != MCP2515_OK) {
        return status;
    }

    status = MCP2515_SetMode(hmcp, config->mode);
    if (status != MCP2515_OK) {
        return status;
    }

    hmcp->initialized = true;

    log_debug("MCP2515: Initialized successfully");

    return MCP2515_OK;
}

MCP2515_Status_t MCP2515_DeInit(MCP2515_Handle_t *hmcp)
{
    MCP2515_Status_t status;

    if (hmcp == NULL) {
        return MCP2515_INVALID_PARAM;
    }
    if (!hmcp->initialized) {
        return MCP2515_NOT_INITIALIZED;
    }

    status = MCP2515_ResetDevice(hmcp);

    hmcp->initialized = false;
    hmcp->device.ready = false;

    return status;
}
