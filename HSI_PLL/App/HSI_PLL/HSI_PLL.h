#ifndef HSI_PLL_H
#define HSI_PLL_H

#include <stdint.h>

/* RCC Register Definitions */
#define RCC_BASE            (0x40023800UL)
#define RCC_CR              (*((volatile uint32_t *)(RCC_BASE + 0x00)))  // Clock Control Register
#define RCC_PLLCFGR         (*((volatile uint32_t *)(RCC_BASE + 0x04)))  // PLL Configuration Register
#define RCC_CFGR            (*((volatile uint32_t *)(RCC_BASE + 0x08)))  // Clock Configuration Register

/* FLASH Register Definitions */
#define FLASH_BASE          (0x40023C00UL)
#define FLASH_ACR           (*((volatile uint32_t *)(FLASH_BASE + 0x00))) // Flash Access Control Register

/* HSI Configuration Bit Masks */
#define RCC_CR_HSION        (1UL << 0)    // HSI clock enable
#define RCC_CR_HSIRDY       (1UL << 1)    // HSI clock ready flag

/* PLL Configuration Bit Masks */
#define RCC_CR_PLLON        (1UL << 24)   // Main PLL enable
#define RCC_CR_PLLRDY       (1UL << 25)   // Main PLL ready flag

/* PLL Source Selection */
#define RCC_PLLCFGR_PLLSRC_HSI  (0UL << 22)  // HSI as PLL source

/* GPIO Register Definitions */
#define GPIOA_BASE          (0x40020000UL)
#define GPIOA_MODER         (*((volatile uint32_t *)(GPIOA_BASE + 0x00)))  // GPIO port mode register
#define GPIOA_AFRH          (*((volatile uint32_t *)(GPIOA_BASE + 0x24)))  // GPIO alternate function high 
#define RCC_AHB1ENR         (*((volatile uint32_t *)(RCC_BASE + 0x30)))  // AHB1 Peripheral Clock Enable Register
#define GPIOA_OSPEEDR       *((volatile uint32_t *)(GPIOA_BASE + 0x08))  // Output speed register

/* Clock Source Selection Masks */
#define RCC_CFGR_SW_PLL     (2UL << 0)    // PLL as system clock
#define RCC_CFGR_SWS_PLL    (2UL << 2)    // PLL used as system clock
#define RCC_CFGR_SW_MASK    (3UL << 0)    // System clock switch mask
#define RCC_CFGR_SWS_MASK   (3UL << 2)    // System clock switch status mask

/* MCO1 Configuration Bits */
#define RCC_CFGR_MCO1_PLL   (3UL << 21)   // PLL clock selected as MCO1 source
#define RCC_CFGR_MCO1_MASK  (3UL << 21)   // MCO1 source selection mask
#define RCC_CFGR_MCO1PRE    (7UL << 24)   // MCO1 prescaler mask


// PWR registers
#define PWR_BASE            0x40007000
#define PWR_CR              *((volatile uint32_t *)(PWR_BASE + 0x00))
#define PWR_CSR             *((volatile uint32_t *)(PWR_BASE + 0x04))
#define RCC_APB1ENR         *((volatile uint32_t *)(RCC_BASE + 0x40))

/* Function Prototypes */
void HSI_PLL_Init(void);
void MCO1_OutputPLL(void);

#endif // HSI_PLL_H