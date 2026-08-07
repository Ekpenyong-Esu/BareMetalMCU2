/**
  ******************************************************************************
  * @file    i2c_core.c
  * @brief   Lifecycle and handle accessor for the I2C bus driver
  ******************************************************************************
  */

#include "i2c_core.h"
#include "log.h"

#define I2C_CLOCK_SPEED_STANDARD     100000U

static I2C_HandleTypeDef s_hi2c3;
static bool s_initialized;

I2C_HandleTypeDef *I2C_GetHandle(void) { return &s_hi2c3; }

I2C_StatusTypeDef I2C_ConvertHALStatus(HAL_StatusTypeDef halStatus)
{
    switch (halStatus) {
        case HAL_OK:      return I2C_OK;
        case HAL_ERROR:   return I2C_ERROR;
        case HAL_BUSY:    return I2C_BUSY;
        case HAL_TIMEOUT: return I2C_TIMEOUT;
        default:          return I2C_ERROR;
    }
}

bool I2C_IsReady(void)
{
    /* Instance is assigned before HAL_I2C_Init runs, so testing it would
       report a bus that failed to come up as usable. */
    return s_initialized;
}

I2C_StatusTypeDef I2C_Init(void)
{
    log_debug("I2C: Initializing I2C3");

    s_hi2c3.Instance = I2C3;
    s_hi2c3.Init.ClockSpeed = I2C_CLOCK_SPEED_STANDARD;
    s_hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
    s_hi2c3.Init.OwnAddress1 = 0;
    s_hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    s_hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    s_hi2c3.Init.OwnAddress2 = 0;
    s_hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    s_hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&s_hi2c3) != HAL_OK) {
        log_error("I2C: I2C3 initialization failed");
        s_initialized = false;
        return I2C_ERROR;
    }

    s_initialized = true;
    log_debug("I2C: I2C3 initialized successfully");

    return I2C_OK;
}

I2C_StatusTypeDef I2C_Init_Custom(const I2C_ConfigTypeDef *config)
{
    I2C_StatusTypeDef status;

    if (config == NULL) {
        return I2C_INVALID_PARAM;
    }

    s_hi2c3.Instance = I2C3;
    s_hi2c3.Init.ClockSpeed = config->ClockSpeed;
    s_hi2c3.Init.DutyCycle = config->DutyCycle;
    s_hi2c3.Init.OwnAddress1 = config->OwnAddress1;
    s_hi2c3.Init.AddressingMode = config->AddressingMode;
    s_hi2c3.Init.DualAddressMode = config->DualAddressMode;
    s_hi2c3.Init.OwnAddress2 = config->OwnAddress2;
    s_hi2c3.Init.GeneralCallMode = config->GeneralCallMode;
    s_hi2c3.Init.NoStretchMode = config->NoStretchMode;

    status = I2C_ConvertHALStatus(HAL_I2C_Init(&s_hi2c3));
    s_initialized = (status == I2C_OK);

    return status;
}

I2C_StatusTypeDef I2C_DeInit(void)
{
    s_initialized = false;

    return I2C_ConvertHALStatus(HAL_I2C_DeInit(&s_hi2c3));
}

I2C_StatusTypeDef I2C_Reinit(void)
{
    I2C_StatusTypeDef status;

    /* HAL_I2C_DeInit leaves the Init struct untouched, so it can be reused. */
    status = I2C_ConvertHALStatus(HAL_I2C_Init(&s_hi2c3));
    s_initialized = (status == I2C_OK);

    return status;
}
