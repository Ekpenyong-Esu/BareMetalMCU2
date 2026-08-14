/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Application 1 - Blink & Heartbeat (STM32F429I-DISC1)
 ******************************************************************************
 */
/* USER CODE END Header */

#include "main.h"
#include "sys.h"
#include "blink_heartbeat_freertos_semaphores.h"

/**
 * @brief  Program entry point: bring up the system, then run Application 2
 *         (Blink & Heartbeat on FreeRTOS, mutex + binary semaphore).
 */
int main(void)
{
    SYS_Init();                 /* HAL init, system clock, SysTick */
    BlinkHeartbeatFreeRTOSSemaphores_Run(); /* Starts FreeRTOS scheduler; never returns */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    printf("Wrong parameters value: file %s on line %lu\r\n", file, (unsigned long)line);
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
