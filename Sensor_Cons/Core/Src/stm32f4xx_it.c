/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"
#include "uart_core.h"
#include "rtc.h"
#include "mic_events.h"
#include "audio_core.h"
#include "dma2d_core.h"
#include "ltdc_core.h"
#include "rng_async.h"
#include "wwdg_ewi.h"
#include "eth_irq.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "SEGGER_SYSVIEW.h"

/* USER CODE END Includes */
/**
  * @brief This function handles System tick timer.
  */


/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* Note: SysTick is reserved for FreeRTOS.
 * HAL timebase uses TIM6 (see stm32f4xx_hal_timebase_tim.c)
 * LVGL uses HAL_GetTick() via LV_TICK_CUSTOM */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern HCD_HandleTypeDef hhcd_USB_OTG_HS;
extern TIM_HandleTypeDef htim6;
/* DMA handles */
extern DMA_HandleTypeDef hdma_uart1_tx;  /* UART TX DMA handle */
extern DMA_HandleTypeDef hdma_uart1_rx;  /* UART RX DMA handle */

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles TIM6 global interrupt, DAC1 and DAC2 underrun error interrupts.
  */
void TIM6_DAC_IRQHandler(void)
{
  /* USER CODE BEGIN TIM6_DAC_IRQn 0 */
  SEGGER_SYSVIEW_RecordEnterISR();
  /* USER CODE END TIM6_DAC_IRQn 0 */
  HAL_TIM_IRQHandler(&htim6);
  /* USER CODE BEGIN TIM6_DAC_IRQn 1 */
  SEGGER_SYSVIEW_RecordExitISR();
  /* USER CODE END TIM6_DAC_IRQn 1 */

  /* USER CODE END TIM6_DAC_IRQn 1 */
}

/**
  * @brief This function handles USB On The Go HS global interrupt.
  */
void OTG_HS_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_HS_IRQn 0 */

  /* USER CODE END OTG_HS_IRQn 0 */
  HAL_HCD_IRQHandler(&hhcd_USB_OTG_HS);
  /* USER CODE BEGIN OTG_HS_IRQn 1 */

  /* USER CODE END OTG_HS_IRQn 1 */
}

/**
  * @brief This function handles LTDC global interrupt.
  */
void LTDC_IRQHandler(void)
{
  /* USER CODE BEGIN LTDC_IRQn 0 */

  /* USER CODE END LTDC_IRQn 0 */
  LTDC_ISR_Dispatch();
  /* USER CODE BEGIN LTDC_IRQn 1 */

  /* USER CODE END LTDC_IRQn 1 */
}

/**
  * @brief This function handles DMA2D global interrupt.
  */
void DMA2D_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2D_IRQn 0 */

  /* USER CODE END DMA2D_IRQn 0 */
  DMA2D_ISR_Dispatch();
  /* USER CODE BEGIN DMA2D_IRQn 1 */

  /* USER CODE END DMA2D_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[15:10] interrupts.
  */
void EXTI15_10_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
}

/**
  * @brief This function handles EXTI line0 interrupt.
  */
void EXTI0_IRQHandler(void)
{
  SEGGER_SYSVIEW_RecordEnterISR();
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
  SEGGER_SYSVIEW_RecordExitISR();
}

/**
  * @brief This function handles USART1 global interrupt (UART interrupt mode).
  */
void USART1_IRQHandler(void)
{
  UART_Handle_t *uart = UART_GetActiveHandle();

  if (uart != NULL && uart->huart != NULL && uart->huart->Instance == USART1) {
    HAL_UART_IRQHandler(uart->huart);
  }
}

/**
  * @brief This function handles DMA2 stream5 global interrupt (UART1 RX).
  */
void DMA2_Stream5_IRQHandler(void)
{
  UART_Handle_t *uart = UART_GetActiveHandle();

  if (uart != NULL && uart->huart != NULL && uart->huart->hdmarx != NULL) {
    HAL_DMA_IRQHandler(uart->huart->hdmarx);
  }
}

/**
  * @brief This function handles DMA2 stream7 global interrupt (UART1 TX).
  */
void DMA2_Stream7_IRQHandler(void)
{
  UART_Handle_t *uart = UART_GetActiveHandle();

  if (uart != NULL && uart->huart != NULL && uart->huart->hdmatx != NULL) {
    HAL_DMA_IRQHandler(uart->huart->hdmatx);
  }
}

/**
  * @brief This function handles RTC Alarm (A and B) through EXTI line.
  *        Deferred to the RTC driver via RTC_ISR_Dispatch().
  */
void RTC_Alarm_IRQHandler(void)
{
  RTC_ISR_Dispatch();
}

/**
  * @brief This function handles DMA1 stream3 global interrupt (MIC PDM RX).
  *        Deferred to the MIC driver via MIC_DMA_IRQHandler().
  */
void DMA1_Stream3_IRQHandler(void)
{
  MIC_DMA_IRQHandler();
}

/**
  * @brief This function handles SPI2/I2S2 global interrupt (MIC PDM audio).
  *        Deferred to the MIC driver via MIC_I2S_IRQHandler().
  */
void SPI2_IRQHandler(void)
{
  MIC_I2S_IRQHandler();
}

/**
  * @brief This function handles DMA2 stream3 global interrupt (AUDIO SAI1_A TX).
  *        Deferred to the audio driver via AUDIO_IRQHandler().
  */
void DMA2_Stream3_IRQHandler(void)
{
  AUDIO_IRQHandler();
}

/**
  * @brief This function handles DMA1 stream7 global interrupt (AUDIO I2S3 TX).
  *        Deferred to the audio driver via AUDIO_IRQHandler().
  */
void DMA1_Stream7_IRQHandler(void)
{
  AUDIO_IRQHandler();
}

/**
  * @brief This function handles the shared HASH and RNG global interrupt.
  *        Deferred to the RNG driver via RNG_IRQHandler().
  */
void HASH_RNG_IRQHandler(void)
{
  RNG_IRQHandler();
}

/**
  * @brief This function handles the Window Watchdog early wakeup interrupt.
  *        Deferred to the WWDG driver via WWDG_EWI_IRQHandler().
  */
void WWDG_IRQHandler(void)
{
  WWDG_EWI_IRQHandler();
}

/**
  * @brief This function handles the Ethernet global interrupt.
  *        Deferred to the ETH driver via ETH_Driver_IRQHandler().
  */
void ETH_IRQHandler(void)
{
  ETH_Driver_IRQHandler();
}

/**
  * @brief This function handles System tick timer.
  */
// void SysTick_Handler(void)
// {
//   SEGGER_SYSVIEW_RecordEnterISR();
//   HAL_SYSTICK_IRQHandler();
//   SEGGER_SYSVIEW_RecordExitISR();
// }

/* USER CODE BEGIN 1 */


/* USER CODE END 1 */
