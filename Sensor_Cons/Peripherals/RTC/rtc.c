/**
 ******************************************************************************
 * @file    rtc.c
 * @author  Mahonri
 * @brief   RTC lifecycle: clock source, initialisation and handle ownership
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "rtc_core.h"
#include "log.h"

/* Private define ------------------------------------------------------------*/

/** LSI runs at ~32 kHz: 32000 / ((124 + 1) * (255 + 1)) = 1 Hz */
#define RTC_ASYNCH_PREDIV       124U
#define RTC_SYNCH_PREDIV        255U

/* Private variables ---------------------------------------------------------*/
static RTC_HandleTypeDef hrtc;

/* Private function prototypes -----------------------------------------------*/
static RTC_StatusTypeDef RTC_MspInit(void);
static void RTC_MspDeInit(void);

/* Public functions ----------------------------------------------------------*/

RTC_StatusTypeDef RTC_Init(void)
{
    log_debug("RTC: Initializing RTC");

    if (RTC_MspInit() != RTC_STATUS_OK)
    {
        log_error("RTC: Clock source configuration failed");
        return RTC_STATUS_ERROR;
    }

    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
    hrtc.Init.SynchPrediv = RTC_SYNCH_PREDIV;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

    if (HAL_RTC_Init(&hrtc) != HAL_OK)
    {
        return RTC_STATUS_ERROR;
    }

    log_debug("RTC: RTC initialized successfully");

    return RTC_STATUS_OK;
}

RTC_StatusTypeDef RTC_DeInit(void)
{
    if (HAL_RTC_DeInit(&hrtc) != HAL_OK)
    {
        return RTC_STATUS_ERROR;
    }

    RTC_MspDeInit();

    return RTC_STATUS_OK;
}

RTC_HandleTypeDef* RTC_GetHandle(void)
{
    return &hrtc;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Select and start the RTC clock source
 * @note   LSI is used rather than LSE because the Discovery board has no
 *         guaranteed 32.768 kHz crystal fitted.
 * @retval RTC_StatusTypeDef: Status of the operation
 */
static RTC_StatusTypeDef RTC_MspInit(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        return RTC_STATUS_ERROR;
    }

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        return RTC_STATUS_ERROR;
    }

    __HAL_RCC_RTC_ENABLE();

    return RTC_STATUS_OK;
}

/**
 * @brief  Stop the RTC peripheral clock
 * @retval None
 */
static void RTC_MspDeInit(void)
{
    __HAL_RCC_RTC_DISABLE();
}
