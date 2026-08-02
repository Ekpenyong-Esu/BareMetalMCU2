/**
  ******************************************************************************
  * @file    sys.c
  * @brief   System module implementation
  * @details This file provides code for core system configuration
  *          including clock setup and error handling.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "sys.h"
#include "log.h"

/* Private variables ---------------------------------------------------------*/
static uint32_t s_cyclesPerMicrosecond = 1U;

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Start the DWT cycle counter used as microsecond time base
  */
static void SYS_MicrosInit(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0U;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  s_cyclesPerMicrosecond = SystemCoreClock / 1000000U;
  if (s_cyclesPerMicrosecond == 0U)
  {
    s_cyclesPerMicrosecond = 1U;
  }
}

/**
  * @brief  System Initialization Function
  * @details Performs fundamental system initialization by:
  *          1. Initializing the HAL library
  *          2. Configuring the Flash interface and SysTick
  *          3. Setting up the system clock
  *
  * @note   This function should be called at the beginning of the
  *         application before any other initialization
  * @param  None
  * @retval None
  */
void SYS_Init(void)
{
  log_debug("SYS: Initializing system");

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Microsecond time base (needs the final core clock) */
  SYS_MicrosInit();

  log_debug("SYS: System initialized successfully");
}

/**
  * @brief  Free-running microsecond counter
  * @retval Elapsed microseconds since SYS_Init()
  */
uint32_t SYS_GetMicros(void)
{
  return DWT->CYCCNT / s_cyclesPerMicrosecond;
}

/**
  * @brief  System Clock Configuration
  * @details Configures the STM32F429 clock system with the following settings:
  *          - HSE used as PLL source (8 MHz external crystal)
  *          - PLL configured for 168 MHz system clock
  *          - Flash latency set to 5 wait states for 168 MHz
  *          - AHB bus running at system clock frequency (168 MHz)
  *          - APB1 peripheral clock at 42 MHz (HCLK/4)
  *          - APB2 peripheral clock at 84 MHz (HCLK/2)
  *          - Power regulator output voltage scale 1 (highest performance)
  *
  * @note   This configuration provides maximum CPU performance
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;              /* 8 MHz HSE / 4 = 2 MHz PLL input */
  RCC_OscInitStruct.PLL.PLLN = 168;            /* 2 MHz * 168 = 336 MHz VCO */
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;  /* 336 MHz / 2 = 168 MHz SYSCLK */
  RCC_OscInitStruct.PLL.PLLQ = 7;              /* 336 MHz / 7 = 48 MHz for USB OTG FS */

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;  /* HCLK  = 168 MHz */
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;   /* PCLK1 = 42 MHz (APB1 max) */
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;   /* PCLK2 = 84 MHz (APB2 max) */

  /* 5 wait states is the minimum for 168 MHz at voltage scale 1 */
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  Error Handler Function
  * @details Halts the CPU with interrupts disabled when a critical
  *          initialization step fails.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
