#include "HSI.h"

/**
 * @brief Initialize HSI clock (16MHz)
 */
void HSI_Init(void) {
    // Enable HSI clock
    RCC_CR |= RCC_CR_HSION;

    // Wait until HSI is stable
    while(!(RCC_CR & RCC_CR_HSIRDY));

    // Select HSI as system clock
    RCC_CFGR = (RCC_CFGR & ~RCC_CFGR_SW_MASK) | RCC_CFGR_SW_HSI;

    // Wait until HSI is used as the system clock
    while((RCC_CFGR & RCC_CFGR_SWS_MASK) != RCC_CFGR_SWS_HSI);
}


/**
 * @brief Configure MCO1 to output HSI clock on PA8
 *
 * This function:
 * 1. Enables GPIOA clock
 * 2. Configures PA8 as alternate function (AF0 for MCO1)
 * 3. Sets MCO1 source as HSI and configures prescaler
 */
 void MCO1_OutputHSI(void) {
    // Enable GPIOA clock
    RCC_AHB1ENR |= (1UL << 0);

    // Configure PA8 in alternate function mode
    GPIOA_MODER &= ~(3UL << (8 * 2));  // Clear PA8 mode bits
    GPIOA_MODER |= (2UL << (8 * 2));   // Set PA8 to alternate function mode

    // Configure PA8 alternate function as AF0 (MCO1)
    // PA8 is controlled by AFRH (pins 8-15)
    GPIOA_AFRH &= ~(0xFUL << ((8 - 8) * 4));  // Clear AF selection for PA8
    // No need to set bits for AF0 as it's 0x0

    // Configure MCO1 to output HSI
    RCC_CFGR &= ~RCC_CFGR_MCO1_MASK;  // Clear MCO1 source selection bits
    RCC_CFGR |= RCC_CFGR_MCO1_HSI;    // Select HSI as source for MCO1

    // Set MCO1 prescaler (no division for max speed)
    //RCC_CFGR &= ~RCC_CFGR_MCO1PRE;    // No division (divide by 1)
    // Set MCO1 prescaler to divide by 2
    RCC_CFGR |= (6UL << 24);          // Set prescaler to divide by 2
}
