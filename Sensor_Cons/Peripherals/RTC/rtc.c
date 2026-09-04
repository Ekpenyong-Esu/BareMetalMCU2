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
#define RTC_LSI_ASYNCH_PREDIV 124U
#define RTC_LSI_SYNCH_PREDIV 255U

/** LSE is a 32.768 kHz crystal: 32768 / ((127 + 1) * (255 + 1)) = 1 Hz */
#define RTC_LSE_ASYNCH_PREDIV 127U
#define RTC_LSE_SYNCH_PREDIV 255U

/* Private variables ---------------------------------------------------------*/
static RTC_HandleTypeDef hrtc;

/* Private function prototypes -----------------------------------------------*/
static RTC_StatusTypeDef RTC_ValidateConfig(const RTC_ConfigTypeDef *config);
static RTC_StatusTypeDef RTC_MspInit(const RTC_ConfigTypeDef *config);
static RTC_StatusTypeDef RTC_StartOscillator(RTC_ClockSourceTypeDef source);
static uint32_t RTC_ClockSelection(const RTC_ConfigTypeDef *config);
static void RTC_MspDeInit(void);

/* Public functions ----------------------------------------------------------*/

RTC_ConfigTypeDef RTC_ConfigForClockSource(RTC_ClockSourceTypeDef source) {
    RTC_ConfigTypeDef config = {0};

    config.ClockSource = source;
    config.HourFormat = RTC_HOURFORMAT_24;

    switch (source) {
        case RTC_CLOCK_LSI:
            config.AsynchPrediv = RTC_LSI_ASYNCH_PREDIV;
            config.SynchPrediv = RTC_LSI_SYNCH_PREDIV;
            break;

        case RTC_CLOCK_LSE:
            config.AsynchPrediv = RTC_LSE_ASYNCH_PREDIV;
            config.SynchPrediv = RTC_LSE_SYNCH_PREDIV;
            break;

        case RTC_CLOCK_HSE_DIV:
        default:
            /* Depends on the crystal fitted; the caller fills these in. */
            break;
    }

    return config;
}

RTC_ConfigTypeDef RTC_ConfigDefault(void) {
    return RTC_ConfigForClockSource(RTC_CLOCK_LSI);
}

RTC_StatusTypeDef RTC_Init_Custom(const RTC_ConfigTypeDef *config) {
    if (RTC_ValidateConfig(config) != RTC_STATUS_OK) {
        return RTC_STATUS_ERROR;
    }

    log_debug("RTC: Initializing RTC");

    if (RTC_MspInit(config) != RTC_STATUS_OK) {
        log_error("RTC: Clock source configuration failed");
        return RTC_STATUS_ERROR;
    }

    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = config->HourFormat;
    hrtc.Init.AsynchPrediv = config->AsynchPrediv;
    hrtc.Init.SynchPrediv = config->SynchPrediv;

    /* The calibration output pin is shared with other functions on most
       boards, so it stays off rather than becoming a setting that could
       silently claim a pin. */
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

    if (HAL_RTC_Init(&hrtc) != HAL_OK) {
        return RTC_STATUS_ERROR;
    }

    log_debug("RTC: RTC initialized successfully");

    return RTC_STATUS_OK;
}

RTC_StatusTypeDef RTC_Init(void) {
    const RTC_ConfigTypeDef config = RTC_ConfigDefault();

    return RTC_Init_Custom(&config);
}

RTC_StatusTypeDef RTC_DeInit(void) {
    if (HAL_RTC_DeInit(&hrtc) != HAL_OK) {
        return RTC_STATUS_ERROR;
    }

    RTC_MspDeInit();

    return RTC_STATUS_OK;
}

RTC_HandleTypeDef *RTC_GetHandle(void) {
    return &hrtc;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Reject configurations the hardware cannot honour
 * @param  config Configuration to check
 * @retval RTC_StatusTypeDef: RTC_STATUS_OK when usable
 */
static RTC_StatusTypeDef RTC_ValidateConfig(const RTC_ConfigTypeDef *config) {
    if (config == NULL) {
        return RTC_STATUS_ERROR;
    }

    switch (config->ClockSource) {
        case RTC_CLOCK_LSI:
        case RTC_CLOCK_LSE:
            return RTC_STATUS_OK;

        case RTC_CLOCK_HSE_DIV:
            /* The divider has no sensible default: it depends on the crystal. */
            return (config->HseRtcClockSelection != 0U) ? RTC_STATUS_OK : RTC_STATUS_ERROR;

        default:
            return RTC_STATUS_ERROR;
    }
}

/**
 * @brief  Start the chosen oscillator and route it to the RTC
 * @param  config Configuration naming the clock source
 * @retval RTC_StatusTypeDef: Status of the operation
 */
static RTC_StatusTypeDef RTC_MspInit(const RTC_ConfigTypeDef *config) {
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    if (RTC_StartOscillator(config->ClockSource) != RTC_STATUS_OK) {
        return RTC_STATUS_ERROR;
    }

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RTC_ClockSelection(config);
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        return RTC_STATUS_ERROR;
    }

    __HAL_RCC_RTC_ENABLE();

    return RTC_STATUS_OK;
}

/**
 * @brief  Bring the low-speed oscillator up, or check HSE is already running
 * @note   HSE is the system clock's business: starting or stopping it here
 *         could take the CPU clock down, so it is only verified.
 * @param  source Oscillator to start
 * @retval RTC_StatusTypeDef: Status of the operation
 */
static RTC_StatusTypeDef RTC_StartOscillator(RTC_ClockSourceTypeDef source) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

    switch (source) {
        case RTC_CLOCK_LSI:
            RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
            RCC_OscInitStruct.LSIState = RCC_LSI_ON;
            break;

        case RTC_CLOCK_LSE:
            RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
            RCC_OscInitStruct.LSEState = RCC_LSE_ON;
            break;

        case RTC_CLOCK_HSE_DIV:
        default:
            return __HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY) ? RTC_STATUS_OK : RTC_STATUS_ERROR;
    }

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        return RTC_STATUS_ERROR;
    }

    return RTC_STATUS_OK;
}

/**
 * @brief  RCC selector value for the configured clock source
 * @param  config Configuration naming the clock source
 * @retval uint32_t: One of RCC_RTCCLKSOURCE_*
 */
static uint32_t RTC_ClockSelection(const RTC_ConfigTypeDef *config) {
    switch (config->ClockSource) {
        case RTC_CLOCK_LSE:
            return RCC_RTCCLKSOURCE_LSE;

        case RTC_CLOCK_HSE_DIV:
            return config->HseRtcClockSelection;

        case RTC_CLOCK_LSI:
        default:
            return RCC_RTCCLKSOURCE_LSI;
    }
}

/**
 * @brief  Stop the RTC peripheral clock
 * @retval None
 */
static void RTC_MspDeInit(void) {
    __HAL_RCC_RTC_DISABLE();
}
