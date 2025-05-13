#include "HSE_PLL.h"

/**
 * @brief Initialize HSE with PLL to reach 180MHz system clock
 *
 * This function:
 * 1. Enables HSE (8MHz EXternal oscillator)
 * 2. Configures PLL to use HSE as input
 * 3. Sets up PLL for 180MHz output
 * 4. Configures flash wait states
 * 5. Switches system clock to PLL
 *
 * PLL configuration:
 * - Input: HSE (8 MHz)
 * - PLLM = 8 (divides to 1 MHz)
 * - PLLN = 360 (multiplies to 360 MHz)
 * - PLLP = 2 (divides to 180 MHz system clock)
 */
void HSE_PLL_Init(void) {
    // 1. Enable HSE clock
    RCC_CR |= RCC_CR_HSEON;

    // Wait until HSE is stable
    while(!(RCC_CR & RCC_CR_HSERDY));

    // 2. Configure Flash latency for high frequency
    FLASH_ACR &= ~0x0F;               // Clear latency bits
    FLASH_ACR |= 0x05;                // 5 wait states for 180MHz
    FLASH_ACR |= (1 << 8) |           // Enable prefetch
                 (1 << 9) |           // Enable instruction cache
                 (1 << 10);           // Enable data cache


    // 3. Disable PLL before configuration
    RCC_CR &= ~RCC_CR_PLLON;

    // Wait until PLL is off
    while(RCC_CR & RCC_CR_PLLRDY);

    // 4. Configure PLL
    // HSE = 8MHz, /M=8, *N=360, /P=2 → 180MHz
    RCC_PLLCFGR = RCC_PLLCFGR_PLLSRC_HSE | // HSE as source
                  (8 << 0) |               // PLLM = 8
                  (360 << 6) |              // PLLN = 360
                  (0 << 16) |               // PLLP = 2 (00 = 2)
                  (7 << 24);                // PLLQ = 7 (for USB, etc.)


    // 5. Enable PLL
    RCC_CR |= RCC_CR_PLLON;

    // Wait until PLL is stable
    while(!(RCC_CR & RCC_CR_PLLRDY));


    // Add after PLL is stable but before switching system clock
    // Enable PWR clock
    RCC_APB1ENR |= (1 << 28);  // PWR clock enable

    // Set voltage scaling to Range 1
    PWR_CR &= ~(3 << 14);      // Clear VOS bits
    PWR_CR |= (3 << 14);       // Set VOS to Range 1 (11)

    // Enable Over-drive mode
    PWR_CR |= (1 << 16);       // Enable overdrive
    while(!(PWR_CSR & (1 << 16))); // Wait for ODRDY

    // Enable Over-drive switching
    PWR_CR |= (1 << 17);       // Enable overdrive switching
    while(!(PWR_CSR & (1 << 17))); // Wait for ODSWRDY

    // 6. Configure bus prescalers
    RCC_CFGR &= ~(0xF << 4);          // Clear AHB prescaler
    RCC_CFGR &= ~(0x7 << 10);         // Clear APB1 prescaler
    RCC_CFGR &= ~(0x7 << 13);         // Clear APB2 prescaler

    RCC_CFGR |= (0 << 4);             // AHB prescaler = 1 (180MHz)
    RCC_CFGR |= (5 << 10);            // APB1 prescaler = 4 (45MHz)
    RCC_CFGR |= (4 << 13);            // APB2 prescaler = 2 (90MHz)

    // 7. Select PLL as system clock source
    RCC_CFGR &= ~RCC_CFGR_SW_MASK;    // Clear SW bits
    RCC_CFGR |= RCC_CFGR_SW_PLL;      // Set PLL as system clock

    // 8. Wait until PLL is used as system clock
    while((RCC_CFGR & RCC_CFGR_SWS_MASK) != RCC_CFGR_SWS_PLL);
}

/**
 * @brief Configure MCO1 to output PLL clock on PA8
 *
 * This function:
 * 1. Enables GPIOA clock
 * 2. Configures PA8 as AF0 (MCO1)
 * 3. Sets MCO1 to output PLL clock with prescaler
 */
void MCO1_OutputPLL(void) {
    // 1. Enable GPIOA clock
    RCC_AHB1ENR |= (1UL << 0);

    // 2. Configure PA8 in alternate function mode
    GPIOA_MODER &= ~(3UL << (8 * 2));     // Clear mode bits for PA8
    GPIOA_MODER |= (2UL << (8 * 2));      // Set PA8 to alternate function

    // Set PA8 to very high speed - ADD THIS SECTION
    GPIOA_OSPEEDR &= ~(3UL << (8 * 2));   // Clear speed bits for PA8
    GPIOA_OSPEEDR |= (3UL << (8 * 2));    // Set PA8 to very high speed (11)

    // 3. Configure PA8 alternate function to AF0 (MCO1)
    GPIOA_AFRH &= ~(0xFUL << ((8 - 8) * 4)); // Clear AF bits for PA8
    // No need to set bits for AF0 (MCO1) as it's 0x0

    // 4. Configure MCO1 to output PLL clock
    RCC_CFGR &= ~RCC_CFGR_MCO1_MASK;      // Clear MCO1 source selection bits
    RCC_CFGR |= RCC_CFGR_MCO1_PLL;        // Set PLL as MCO1 source

    // 5. Configure MCO1 prescaler to divide by 5 (180MHz/5 = 36MHz)
    RCC_CFGR &= ~RCC_CFGR_MCO1PRE;        // Clear MCO1 prescaler bits
    RCC_CFGR |= (7UL << 24);              // Set prescaler to 5 (111)
}
