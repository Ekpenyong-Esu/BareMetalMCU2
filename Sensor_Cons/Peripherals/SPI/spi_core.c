/**
  ******************************************************************************
  * @file    spi_core.c
  * @brief   Lifecycle and handle accessor for the SPI bus driver
  ******************************************************************************
  */

#include "spi_core.h"
#include "log.h"

#define SPI_CRC_POLYNOMIAL_DEFAULT    10U

static SPI_HandleTypeDef s_hspi5;
static bool s_initialized;

SPI_HandleTypeDef *SPI_GetHandle(void) { return &s_hspi5; }

SPI_StatusTypeDef SPI_ConvertHALStatus(HAL_StatusTypeDef halStatus)
{
    switch (halStatus) {
        case HAL_OK:      return SPI_OK;
        case HAL_ERROR:   return SPI_ERROR;
        case HAL_BUSY:    return SPI_BUSY;
        case HAL_TIMEOUT: return SPI_TIMEOUT;
        default:          return SPI_ERROR;
    }
}

bool SPI_IsReady(void)
{
    /* Instance is set before HAL_SPI_Init runs, so testing it would hide init failures. */
    return s_initialized;
}

SPI_StatusTypeDef SPI_Init(void)
{
    log_debug("SPI: Initializing SPI5");

    s_hspi5.Instance = SPI5;
    s_hspi5.Init.Mode = SPI_MODE_MASTER;
    s_hspi5.Init.Direction = SPI_DIRECTION_2LINES;
    s_hspi5.Init.DataSize = SPI_DATASIZE_8BIT;
    s_hspi5.Init.CLKPolarity = SPI_POLARITY_LOW;
    s_hspi5.Init.CLKPhase = SPI_PHASE_1EDGE;
    s_hspi5.Init.NSS = SPI_NSS_SOFT;
    s_hspi5.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    s_hspi5.Init.FirstBit = SPI_FIRSTBIT_MSB;
    s_hspi5.Init.TIMode = SPI_TIMODE_DISABLE;
    s_hspi5.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    s_hspi5.Init.CRCPolynomial = SPI_CRC_POLYNOMIAL_DEFAULT;

    if (HAL_SPI_Init(&s_hspi5) != HAL_OK) {
        log_error("SPI: SPI5 initialization failed");
        s_initialized = false;
        return SPI_ERROR;
    }

    s_initialized = true;
    log_debug("SPI: SPI5 initialized successfully");

    return SPI_OK;
}

SPI_StatusTypeDef SPI_Init_Custom(const SPI_ConfigTypeDef *config)
{
    SPI_StatusTypeDef status = SPI_OK;
    HAL_StatusTypeDef halStatus = HAL_OK;

    if (config == NULL) {
        return SPI_INVALID_PARAM;
    }

    s_hspi5.Instance = SPI5;
    s_hspi5.Init.Mode = config->Mode;
    s_hspi5.Init.Direction = config->Direction;
    s_hspi5.Init.DataSize = config->DataSize;
    s_hspi5.Init.CLKPolarity = config->CLKPolarity;
    s_hspi5.Init.CLKPhase = config->CLKPhase;
    s_hspi5.Init.NSS = config->NSS;
    s_hspi5.Init.BaudRatePrescaler = config->BaudRatePrescaler;
    s_hspi5.Init.FirstBit = config->FirstBit;
    s_hspi5.Init.TIMode = config->TIMode;
    s_hspi5.Init.CRCCalculation = config->CRCCalculation;
    s_hspi5.Init.CRCPolynomial = config->CRCPolynomial;

    halStatus = HAL_SPI_Init(&s_hspi5);
    status = SPI_ConvertHALStatus(halStatus);

    s_initialized = (status == SPI_OK);

    return status;
}

SPI_StatusTypeDef SPI_DeInit(void)
{
    HAL_StatusTypeDef halStatus = HAL_SPI_DeInit(&s_hspi5);

    s_initialized = false;

    return SPI_ConvertHALStatus(halStatus);
}

SPI_StatusTypeDef SPI_Reinit(void)
{
    SPI_StatusTypeDef status = SPI_OK;
    HAL_StatusTypeDef halStatus = HAL_ERROR;

    /* HAL_SPI_DeInit leaves the Init struct untouched, so it can be reused. */
    halStatus = HAL_SPI_Init(&s_hspi5);
    status = SPI_ConvertHALStatus(halStatus);
    s_initialized = (status == SPI_OK);

    return status;
}
