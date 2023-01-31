/*
 * main.h
 *
 *  Created on: Jan 28, 2023
 *      Author: mahon
 */

#ifndef INC_MAIN_H_
#define INC_MAIN_H_

#include "stm32f4xx_hal.h"



void HAL_MspInit(void)
{
 //Here will do low level processor specific inits.


	//1. Set up the priority grouping of the arm cortex mx processor
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

	//2. Enable the required system exceptions of the arm cortex mx processor
	// SCB -> is called system control block
	// SHCSR -> system handler control and state register it is found in cortx m4 dgug reference manual
	// It is in handling memory fault, bus fault, and usage fault
	SCB->SHCSR |= 0x7 << 16; //usage fault, memory fault and bus fault system exceptions

	//3. configure the priority for the system exceptions
	HAL_NVIC_SetPriority(MemoryManagement_IRQn,0,0);
	HAL_NVIC_SetPriority(BusFault_IRQn,0,0);
	HAL_NVIC_SetPriority(UsageFault_IRQn,0,0);
}



#endif /* INC_MAIN_H_ */
