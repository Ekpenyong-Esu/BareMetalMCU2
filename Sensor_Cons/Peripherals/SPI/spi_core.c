/**
 ******************************************************************************
 * @file    spi_core.c
 * @brief   Bus lifecycle, device registration and ownership for SPI
 ******************************************************************************
 */

#include "spi_core.h"
#include "gpio.h"
#include "log.h"

#define SPI_CRC_POLYNOMIAL_DEFAULT 10U

/* ---- Instance facts: which clock gate, which alternate function ---------- */

static bool SPI_ClockEnable(const SPI_TypeDef *instance) {
    if (instance == SPI1) {
        __HAL_RCC_SPI1_CLK_ENABLE();
    }
    else if (instance == SPI2) {
        __HAL_RCC_SPI2_CLK_ENABLE();
    }
    else if (instance == SPI3) {
        __HAL_RCC_SPI3_CLK_ENABLE();
#ifdef SPI4
    }
    else if (instance == SPI4) {
        __HAL_RCC_SPI4_CLK_ENABLE();
#endif
#ifdef SPI5
    }
    else if (instance == SPI5) {
        __HAL_RCC_SPI5_CLK_ENABLE();
#endif
#ifdef SPI6
    }
    else if (instance == SPI6) {
        __HAL_RCC_SPI6_CLK_ENABLE();
#endif
    }
    else {
        return false;
    }

    return true;
}

static void SPI_ClockDisable(const SPI_TypeDef *instance) {
    if (instance == SPI1) {
        __HAL_RCC_SPI1_CLK_DISABLE();
    }
    else if (instance == SPI2) {
        __HAL_RCC_SPI2_CLK_DISABLE();
    }
    else if (instance == SPI3) {
        __HAL_RCC_SPI3_CLK_DISABLE();
#ifdef SPI4
    }
    else if (instance == SPI4) {
        __HAL_RCC_SPI4_CLK_DISABLE();
#endif
#ifdef SPI5
    }
    else if (instance == SPI5) {
        __HAL_RCC_SPI5_CLK_DISABLE();
#endif
#ifdef SPI6
    }
    else if (instance == SPI6) {
        __HAL_RCC_SPI6_CLK_DISABLE();
#endif
    }
}

/**
 * @brief The usual alternate function for a peripheral's pins
 * @note  A few pins map differently (e.g. PD6 as SPI3_MOSI is AF5), which is
 *        what SPI_BusConfig_t.alternate is for.
 */
static uint8_t SPI_DefaultAlternate(const SPI_TypeDef *instance) {
    if (instance == SPI3) {
        return GPIO_AF6_SPI3;
    }
    return GPIO_AF5_SPI1;
}

/* ---- Pins ---------------------------------------------------------------- */

static void SPI_PinInit(GPIO_TypeDef *port, uint16_t pin, uint32_t pull, uint8_t alternate) {
    GPIO_InitTypeDef init = {0};

    if (port == NULL) {
        return;
    }

    init.Pin = pin;
    init.Mode = GPIO_MODE_AF_PP;
    init.Pull = pull;
    init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    init.Alternate = alternate;
    GPIO_Driver_Pin_Init(port, &init);
}

static void SPI_PinDeInit(GPIO_TypeDef *port, uint16_t pin) {
    if (port != NULL) {
        GPIO_Driver_Pin_DeInit(port, pin);
    }
}

static void SPI_BusInitPins(const SPI_BusConfig_t *config) {
    uint8_t alternate =
        (config->alternate != 0U) ? config->alternate : SPI_DefaultAlternate(config->instance);

    SPI_PinInit(config->sckPort, config->sckPin, config->pull, alternate);
    SPI_PinInit(config->misoPort, config->misoPin, config->pull, alternate);
    SPI_PinInit(config->mosiPort, config->mosiPin, config->pull, alternate);
}

static void SPI_BusDeInitPins(const SPI_BusConfig_t *config) {
    SPI_PinDeInit(config->sckPort, config->sckPin);
    SPI_PinDeInit(config->misoPort, config->misoPin);
    SPI_PinDeInit(config->mosiPort, config->mosiPin);
}

/* ---- Bus ----------------------------------------------------------------- */

bool SPI_BusIsReady(const SPI_Bus_t *bus) {
    return (bus != NULL) && bus->ready;
}

SPI_StatusTypeDef SPI_BusInit(SPI_Bus_t *bus, const SPI_BusConfig_t *config) {
    if (bus == NULL || config == NULL || config->instance == NULL) {
        return SPI_INVALID_PARAM;
    }
    if (config->sckPort == NULL) {
        log_error("SPI: a bus needs at least a clock pin");
        return SPI_INVALID_PARAM;
    }
    if (!SPI_ClockEnable(config->instance)) {
        log_error("SPI: unknown SPI instance");
        return SPI_INVALID_PARAM;
    }

    bus->config = *config;
    bus->hal.Instance = config->instance;
    bus->owner = NULL;

    SPI_BusInitPins(&bus->config);

    bus->ready = true;

    return SPI_OK;
}

SPI_StatusTypeDef SPI_BusDeInit(SPI_Bus_t *bus) {
    if (!SPI_BusIsReady(bus)) {
        return SPI_INVALID_PARAM;
    }

    HAL_StatusTypeDef halStatus = HAL_SPI_DeInit(&bus->hal);

    SPI_BusDeInitPins(&bus->config);
    SPI_ClockDisable(bus->config.instance);

    bus->owner = NULL;
    bus->ready = false;

    return SPI_ConvertHALStatus(halStatus);
}

/* ---- Devices ------------------------------------------------------------- */

SPI_StatusTypeDef SPI_ConvertHALStatus(HAL_StatusTypeDef halStatus) {
    switch (halStatus) {
        case HAL_OK:
            return SPI_OK;
        case HAL_ERROR:
            return SPI_ERROR;
        case HAL_BUSY:
            return SPI_BUSY;
        case HAL_TIMEOUT:
            return SPI_TIMEOUT;
        default:
            return SPI_ERROR;
    }
}

SPI_ConfigTypeDef SPI_ConfigDefault(void) {
    /* Mode 0, MSB first, PCLK/8: what most displays, sensors and memories
       accept as a starting point. */
    const SPI_ConfigTypeDef config = {
        .Mode = SPI_MODE_MASTER,
        .Direction = SPI_DIRECTION_2LINES,
        .DataSize = SPI_DATASIZE_8BIT,
        .CLKPolarity = SPI_POLARITY_LOW,
        .CLKPhase = SPI_PHASE_1EDGE,
        .NSS = SPI_NSS_SOFT,
        .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8,
        .FirstBit = SPI_FIRSTBIT_MSB,
        .TIMode = SPI_TIMODE_DISABLE,
        .CRCCalculation = SPI_CRCCALCULATION_DISABLE,
        .CRCPolynomial = SPI_CRC_POLYNOMIAL_DEFAULT,
    };

    return config;
}

bool SPI_DeviceIsReady(const SPI_Device_t *device) {
    return (device != NULL) && device->ready && SPI_BusIsReady(device->bus);
}

SPI_StatusTypeDef SPI_DeviceInit(SPI_Device_t *device, SPI_Bus_t *bus,
                                 const SPI_ConfigTypeDef *config) {
    if (device == NULL || config == NULL) {
        return SPI_INVALID_PARAM;
    }
    if (!SPI_BusIsReady(bus)) {
        log_error("SPI: device registered on a bus that is not open");
        return SPI_INVALID_PARAM;
    }

    device->bus = bus;
    device->config = *config;
    device->ready = true;

    /* A registration replaces whatever the bus was told last time. */
    if (bus->owner == device) {
        bus->owner = NULL;
    }

    return SPI_OK;
}

SPI_StatusTypeDef SPI_Select(SPI_Device_t *device) {
    if (!SPI_DeviceIsReady(device)) {
        return SPI_INVALID_PARAM;
    }

    SPI_Bus_t *bus = device->bus;

    if (bus->owner == device) {
        return SPI_OK;
    }

    bus->hal.Instance = bus->config.instance;
    bus->hal.Init.Mode = device->config.Mode;
    bus->hal.Init.Direction = device->config.Direction;
    bus->hal.Init.DataSize = device->config.DataSize;
    bus->hal.Init.CLKPolarity = device->config.CLKPolarity;
    bus->hal.Init.CLKPhase = device->config.CLKPhase;
    bus->hal.Init.NSS = device->config.NSS;
    bus->hal.Init.BaudRatePrescaler = device->config.BaudRatePrescaler;
    bus->hal.Init.FirstBit = device->config.FirstBit;
    bus->hal.Init.TIMode = device->config.TIMode;
    bus->hal.Init.CRCCalculation = device->config.CRCCalculation;
    bus->hal.Init.CRCPolynomial = device->config.CRCPolynomial;

    if (HAL_SPI_Init(&bus->hal) != HAL_OK) {
        log_error("SPI: failed to program the bus for the selected device");
        bus->owner = NULL;
        return SPI_ERROR;
    }

    bus->owner = device;

    return SPI_OK;
}

SPI_HandleTypeDef *SPI_GetHandle(const SPI_Device_t *device) {
    if (!SPI_DeviceIsReady(device)) {
        return NULL;
    }
    return &device->bus->hal;
}
