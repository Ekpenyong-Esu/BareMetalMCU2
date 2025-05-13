#ifndef HSE_H
#define HSE_H

#include <stdint.h>

/* RCC Register Definitions */
#define RCC_BASE            (0x40023800UL)
#define RCC_CR              (*((volatile uint32_t *)(RCC_BASE + 0x00)))  // Clock Control Register
#define RCC_PLLCFGR         (*((volatile uint32_t *)(RCC_BASE + 0x04)))  // PLL Configuration Register
#define RCC_CFGR            (*((volatile uint32_t *)(RCC_BASE + 0x08)))  // Clock Configuration Register
#define RCC_AHB1ENR         (*((volatile uint32_t *)(RCC_BASE + 0x30)))  // AHB1 Peripheral Clock Enable Register

/* HSE Configuration Bit Masks */
#define RCC_CR_HSEON        (1UL << 16)   // HSE clock enable
#define RCC_CR_HSERDY       (1UL << 17)   // HSE clock ready flag
#define RCC_CR_HSEBYP       (1UL << 18)   // HSE crystal oscillator bypass

/* Clock Source Selection Masks */
#define RCC_CFGR_SW_HSE     (1UL << 0)    // HSE as system clock
#define RCC_CFGR_SWS_HSE    (1UL << 2)    // HSE used as system clock
#define RCC_CFGR_SW_MASK    (3UL << 0)    // System clock switch mask
#define RCC_CFGR_SWS_MASK   (3UL << 2)    // System clock switch status mask

/* MCO1 Configuration Bits */
#define RCC_CFGR_MCO1_HSE   (2UL << 21)   // HSE clock selected as MCO1 source
#define RCC_CFGR_MCO1_MASK  (3UL << 21)   // MCO1 source selection mask
#define RCC_CFGR_MCO1PRE    (7UL << 24)   // MCO1 prescaler mask

/* GPIO Register Definitions */
#define GPIOA_BASE          (0x40020000UL)
#define GPIOA_MODER         (*((volatile uint32_t *)(GPIOA_BASE + 0x00)))  // GPIO port mode register
#define GPIOA_AFRH          (*((volatile uint32_t *)(GPIOA_BASE + 0x24)))  // GPIO alternate function high register

/* Function Prototypes */
void HSE_Init(void);
void MCO1_OutputHSE(void);

#endif // HSE_H