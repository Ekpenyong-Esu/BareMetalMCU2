/**
 ******************************************************************************
 * @file    i2c_core.c
 * @brief   Bus lifecycle, device registration and ownership for I2C
 ******************************************************************************
 */

#include "i2c_core.h"
#include "gpio.h"
#include "log.h"

#define I2C_CLOCK_SPEED_STANDARD 100000U

/* ---- Instance facts: which clock gate, which alternate function ---------- */

static bool I2C_ClockEnable(const I2C_TypeDef *instance) {
    if (instance == I2C1) {
        __HAL_RCC_I2C1_CLK_ENABLE();
        __HAL_RCC_I2C1_FORCE_RESET();
        __HAL_RCC_I2C1_RELEASE_RESET();
    }
    else if (instance == I2C2) {
        __HAL_RCC_I2C2_CLK_ENABLE();
        __HAL_RCC_I2C2_FORCE_RESET();
        __HAL_RCC_I2C2_RELEASE_RESET();
    }
    else if (instance == I2C3) {
        __HAL_RCC_I2C3_CLK_ENABLE();
        __HAL_RCC_I2C3_FORCE_RESET();
        __HAL_RCC_I2C3_RELEASE_RESET();
    }
    else {
        return false;
    }

    return true;
}

static void I2C_ClockDisable(const I2C_TypeDef *instance) {
    if (instance == I2C1) {
        __HAL_RCC_I2C1_CLK_DISABLE();
    }
    else if (instance == I2C2) {
        __HAL_RCC_I2C2_CLK_DISABLE();
    }
    else if (instance == I2C3) {
        __HAL_RCC_I2C3_CLK_DISABLE();
    }
}

/* Every I2C pin on the F4 is AF4 except a handful (e.g. PB4 as I2C3_SDA is
   AF9), which is what I2C_BusConfig_t.alternate is for. */
#define I2C_DEFAULT_ALTERNATE GPIO_AF4_I2C1

/* ---- Pins ---------------------------------------------------------------- */

static void I2C_PinInit(GPIO_TypeDef *port, uint16_t pin, uint8_t alternate) {
    GPIO_InitTypeDef init = {0};

    init.Pin = pin;
    init.Mode = GPIO_MODE_AF_OD;
    init.Pull = GPIO_PULLUP;
    init.Speed = GPIO_SPEED_FREQ_MEDIUM;
    init.Alternate = alternate;
    GPIO_Driver_Pin_Init(port, &init);
}

void I2C_BusInitPins(const I2C_Bus_t *bus) {
    uint8_t alternate =
        (bus->config.alternate != 0U) ? bus->config.alternate : I2C_DEFAULT_ALTERNATE;

    I2C_PinInit(bus->config.sclPort, bus->config.sclPin, alternate);
    I2C_PinInit(bus->config.sdaPort, bus->config.sdaPin, alternate);
}

/* ---- Bus ----------------------------------------------------------------- */

bool I2C_BusIsReady(const I2C_Bus_t *bus) {
    return (bus != NULL) && bus->ready;
}

I2C_StatusTypeDef I2C_BusInit(I2C_Bus_t *bus, const I2C_BusConfig_t *config) {
    if (bus == NULL || config == NULL || config->instance == NULL || config->sclPort == NULL ||
        config->sdaPort == NULL) {
        return I2C_INVALID_PARAM;
    }
    if (!I2C_ClockEnable(config->instance)) {
        log_error("I2C: unknown I2C instance");
        return I2C_INVALID_PARAM;
    }

    bus->config = *config;
    bus->hal.Instance = config->instance;
    bus->owner = NULL;

    I2C_BusInitPins(bus);

    bus->ready = true;

    return I2C_OK;
}

I2C_StatusTypeDef I2C_BusDeInit(I2C_Bus_t *bus) {
    if (!I2C_BusIsReady(bus)) {
        return I2C_INVALID_PARAM;
    }

    HAL_StatusTypeDef halStatus = HAL_I2C_DeInit(&bus->hal);

    GPIO_Driver_Pin_DeInit(bus->config.sclPort, bus->config.sclPin);
    GPIO_Driver_Pin_DeInit(bus->config.sdaPort, bus->config.sdaPin);
    I2C_ClockDisable(bus->config.instance);

    bus->owner = NULL;
    bus->ready = false;

    return I2C_ConvertHALStatus(halStatus);
}

/* ---- Devices ------------------------------------------------------------- */

I2C_StatusTypeDef I2C_ConvertHALStatus(HAL_StatusTypeDef halStatus) {
    switch (halStatus) {
        case HAL_OK:
            return I2C_OK;
        case HAL_ERROR:
            return I2C_ERROR;
        case HAL_BUSY:
            return I2C_BUSY;
        case HAL_TIMEOUT:
            return I2C_TIMEOUT;
        default:
            return I2C_ERROR;
    }
}

I2C_ConfigTypeDef I2C_ConfigDefault(void) {
    I2C_ConfigTypeDef config;

    config.ClockSpeed = I2C_CLOCK_SPEED_STANDARD;
    config.DutyCycle = I2C_DUTYCYCLE_2;
    config.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    config.OwnAddress1 = 0;
    config.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    config.OwnAddress2 = 0;
    config.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    config.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    return config;
}

bool I2C_DeviceIsReady(const I2C_Device_t *device) {
    return (device != NULL) && device->ready && I2C_BusIsReady(device->bus);
}

I2C_StatusTypeDef I2C_DeviceInit(I2C_Device_t *device, I2C_Bus_t *bus, uint16_t address,
                                 const I2C_ConfigTypeDef *config) {
    if ((device == NULL) || (config == NULL)) {
        return I2C_INVALID_PARAM;
    }
    if (!I2C_BusIsReady(bus)) {
        log_error("I2C: device registered on a bus that is not open");
        return I2C_INVALID_PARAM;
    }

    device->bus = bus;
    device->address = address;
    device->config = *config;
    device->ready = true;

    /* A registration replaces whatever the bus was told last time. */
    if (bus->owner == device) {
        bus->owner = NULL;
    }

    return I2C_OK;
}

I2C_StatusTypeDef I2C_Select(I2C_Device_t *device) {
    if (!I2C_DeviceIsReady(device)) {
        return I2C_INVALID_PARAM;
    }

    I2C_Bus_t *bus = device->bus;

    if (bus->owner == device) {
        return I2C_OK;
    }

    bus->hal.Instance = bus->config.instance;
    bus->hal.Init.ClockSpeed = device->config.ClockSpeed;
    bus->hal.Init.DutyCycle = device->config.DutyCycle;
    bus->hal.Init.AddressingMode = device->config.AddressingMode;
    bus->hal.Init.OwnAddress1 = device->config.OwnAddress1;
    bus->hal.Init.DualAddressMode = device->config.DualAddressMode;
    bus->hal.Init.OwnAddress2 = device->config.OwnAddress2;
    bus->hal.Init.GeneralCallMode = device->config.GeneralCallMode;
    bus->hal.Init.NoStretchMode = device->config.NoStretchMode;

    if (HAL_I2C_Init(&bus->hal) != HAL_OK) {
        log_error("I2C: failed to configure the bus for a device");
        bus->owner = NULL;
        return I2C_ERROR;
    }

    bus->owner = device;

    return I2C_OK;
}

I2C_HandleTypeDef *I2C_GetHandle(const I2C_Device_t *device) {
    if (!I2C_DeviceIsReady(device)) {
        return NULL;
    }
    return &device->bus->hal;
}
