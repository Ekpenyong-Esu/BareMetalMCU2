/*
 * main.c
 *
 *  Created on: Jan 28, 2023
 *      Author: mahon
 */

#include "main.h"


void systemClockConfig();
void UART1_Init();

// Declaring UART1 handle

UART_HandleTypeDef huart1;

int main()
{
	HAL_Init();

	systemClockConfig();
	UART1_Init();

}


void systemClockConfig()
{

}

void UART1_Init()
{

}
