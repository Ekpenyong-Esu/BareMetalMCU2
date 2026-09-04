/**
 * @file    seg_ht1621.c
 * @brief   HT1621 seven-segment backend implementation
 * @details Bit-bangs the 3-wire HT1621 protocol. The controller holds the whole
 *          frame itself, so there is no multiplexing step.
 */

/* Includes ------------------------------------------------------------------*/
#include "seg_ht1621.h"

#include "gpio.h"

/* Private constants ---------------------------------------------------------*/

/** @defgroup HT1621_Commands HT1621 command set
 * @{
 */
#define HT1621_CMD_SYS_DIS 0x00   /**< System disable */
#define HT1621_CMD_SYS_EN 0x01    /**< System enable */
#define HT1621_CMD_LCD_OFF 0x02   /**< LCD off */
#define HT1621_CMD_LCD_ON 0x03    /**< LCD on */
#define HT1621_CMD_TIMER_DIS 0x04 /**< Timer disable */
#define HT1621_CMD_WDT_DIS 0x05   /**< WDT disable */
#define HT1621_CMD_RC_256K 0x18   /**< Internal RC 256K oscillator */
#define HT1621_CMD_BIAS_2_2 0x20  /**< 1/2 bias, 2 commons */
#define HT1621_CMD_BIAS_2_3 0x24  /**< 1/2 bias, 3 commons */
#define HT1621_CMD_BIAS_2_4 0x28  /**< 1/2 bias, 4 commons */
#define HT1621_CMD_BIAS_3_2 0x21  /**< 1/3 bias, 2 commons */
#define HT1621_CMD_BIAS_3_3 0x25  /**< 1/3 bias, 3 commons */
#define HT1621_CMD_BIAS_3_4 0x29  /**< 1/3 bias, 4 commons */
/** @} */

#define HT1621_MODE_COMMAND 0x04 /**< Mode prefix 100, sent MSB first */
#define HT1621_MODE_WRITE 0x05   /**< Mode prefix 101, sent MSB first */
#define HT1621_RAM_CELLS 32      /**< Addressable 4-bit cells */
#define HT1621_CLOCK_HALF_US 2   /**< Half clock period */

/** Iterations of the busy-wait loop below per microsecond at 168MHz (4 cycles each) */
#define HT1621_DELAY_LOOPS_PER_US 42U

/* Private functions ---------------------------------------------------------*/

/** @brief Busy-wait calibrated for a 168MHz core */
static void Seg_Ht1621DelayUs(uint32_t micros) {
    volatile uint32_t count = micros * HT1621_DELAY_LOOPS_PER_US;
    while (count--) {
        __NOP();
    }
}

static void Seg_Ht1621WriteBit(const SegHT1621Pins_t *pins, bool bit) {
    HAL_GPIO_WritePin(pins->wrPort, pins->wrPin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(pins->dataPort, pins->dataPin, bit ? GPIO_PIN_SET : GPIO_PIN_RESET);
    Seg_Ht1621DelayUs(HT1621_CLOCK_HALF_US);

    /* Data is latched on the rising edge */
    HAL_GPIO_WritePin(pins->wrPort, pins->wrPin, GPIO_PIN_SET);
    Seg_Ht1621DelayUs(HT1621_CLOCK_HALF_US);
}

/** @brief Shift out the low @p bitCount bits of @p value, MSB first */
static void Seg_Ht1621WriteBits(const SegHT1621Pins_t *pins, uint8_t value, uint8_t bitCount) {
    for (int8_t i = (int8_t)(bitCount - 1); i >= 0; i--) {
        Seg_Ht1621WriteBit(pins, ((value >> i) & 0x01U) != 0U);
    }
}

static void Seg_Ht1621SendCommand(SegDisplayHandle_t *handle, uint8_t cmd) {
    const SegHT1621Pins_t *pins = &handle->config.config.ht1621.pins;

    HAL_GPIO_WritePin(pins->csPort, pins->csPin, GPIO_PIN_RESET);
    Seg_Ht1621WriteBits(pins, HT1621_MODE_COMMAND, 3);
    Seg_Ht1621WriteBits(pins, cmd, 8);
    Seg_Ht1621WriteBit(pins, false); /* Command frames carry one padding bit */
    HAL_GPIO_WritePin(pins->csPort, pins->csPin, GPIO_PIN_SET);
}

static void Seg_Ht1621SendData(SegDisplayHandle_t *handle, uint8_t addr, uint8_t data) {
    const SegHT1621Pins_t *pins = &handle->config.config.ht1621.pins;

    HAL_GPIO_WritePin(pins->csPort, pins->csPin, GPIO_PIN_RESET);
    Seg_Ht1621WriteBits(pins, HT1621_MODE_WRITE, 3);
    Seg_Ht1621WriteBits(pins, addr, 6);

    /* Data nibbles go out LSB first */
    for (uint8_t i = 0; i < 4; i++) {
        Seg_Ht1621WriteBit(pins, ((data >> i) & 0x01U) != 0U);
    }

    HAL_GPIO_WritePin(pins->csPort, pins->csPin, GPIO_PIN_SET);
}

static uint8_t Seg_Ht1621BiasCommand(uint8_t bias, uint8_t commons) {
    static const uint8_t biasCommands[2][3] = {
        /*        2 commons,           3 commons,           4 commons */
        /* 1/2 */ {HT1621_CMD_BIAS_2_2, HT1621_CMD_BIAS_2_3, HT1621_CMD_BIAS_2_4},
        /* 1/3 */ {HT1621_CMD_BIAS_3_2, HT1621_CMD_BIAS_3_3, HT1621_CMD_BIAS_3_4},
    };

    uint8_t biasIndex = (bias == 2) ? 0U : 1U;
    uint8_t commonIndex = (commons >= 2 && commons <= 4) ? (uint8_t)(commons - 2) : 2U;

    return biasCommands[biasIndex][commonIndex];
}

static void Seg_Ht1621ConfigurePin(GPIO_TypeDef *port, uint16_t pin) {
    GPIO_InitTypeDef init = {0};
    init.Pin = pin;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_Driver_Pin_Init(port, &init);
}

/* Backend operations --------------------------------------------------------*/

static SegStatus_t Seg_Ht1621Init(SegDisplayHandle_t *handle) {
    const SegHT1621Config_t *cfg = &handle->config.config.ht1621;
    const SegHT1621Pins_t *pins = &cfg->pins;

    if (pins->csPort == NULL || pins->wrPort == NULL || pins->dataPort == NULL) {
        return SEG_INVALID_PARAM;
    }

    /* The GPIO driver enables the port clock for each pin it configures */
    Seg_Ht1621ConfigurePin(pins->csPort, pins->csPin);
    Seg_Ht1621ConfigurePin(pins->wrPort, pins->wrPin);
    Seg_Ht1621ConfigurePin(pins->dataPort, pins->dataPin);

    HAL_GPIO_WritePin(pins->csPort, pins->csPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(pins->wrPort, pins->wrPin, GPIO_PIN_SET);

    Seg_Ht1621SendCommand(handle, HT1621_CMD_SYS_EN);
    Seg_Ht1621SendCommand(handle, HT1621_CMD_RC_256K);

    uint8_t biasCmd = Seg_Ht1621BiasCommand(cfg->bias, cfg->commons);
    Seg_Ht1621SendCommand(handle, biasCmd);

    Seg_Ht1621SendCommand(handle, HT1621_CMD_LCD_ON);

    for (uint8_t i = 0; i < HT1621_RAM_CELLS; i++) {
        Seg_Ht1621SendData(handle, i, 0x00);
    }

    return SEG_OK;
}

static void Seg_Ht1621Enable(SegDisplayHandle_t *handle) {
    Seg_Ht1621SendCommand(handle, HT1621_CMD_LCD_ON);
}

static void Seg_Ht1621Disable(SegDisplayHandle_t *handle) {
    Seg_Ht1621SendCommand(handle, HT1621_CMD_LCD_OFF);
}

static void Seg_Ht1621Commit(SegDisplayHandle_t *handle) {
    for (uint8_t i = 0; i < handle->digitCount; i++) {
        Seg_Ht1621SendData(handle, (uint8_t)(i * 2), handle->displayBuffer[i]);
    }
}

const SegDriverOps_t SegHt1621Ops = {
    .init = Seg_Ht1621Init,
    .enable = Seg_Ht1621Enable,
    .disable = Seg_Ht1621Disable,
    .commit = Seg_Ht1621Commit,
    .multiplexStep = NULL,
};
