#ifndef HSI_H
#define HSI_H

#include <stdint.h>

/* RCC Register Definitions */
#define RCC_BASE            (0x40023800UL)
#define RCC_CR              (*((volatile uint32_t *)(RCC_BASE + 0x00)))  // Clock Control Register
#define RCC_CFGR            (*((volatile uint32_t *)(RCC_BASE + 0x08)))  // Clock Configuration Register
#define RCC_AHB1ENR         (*((volatile uint32_t *)(RCC_BASE + 0x30)))  // AHB1 Peripheral Clock Enable Register


/* GPIO Register Definitions */
#define GPIOA_BASE          (0x40020000UL)
#define GPIOA_MODER         (*((volatile uint32_t *)(GPIOA_BASE + 0x00)))  // GPIO port mode register
#define GPIOA_AFRH          (*((volatile uint32_t *)(GPIOA_BASE + 0x24)))  // GPIO alternate function high register

/* HSI Configuration Bit Masks */
#define RCC_CR_HSION        (1UL << 0)    // HSI clock enable
#define RCC_CR_HSIRDY       (1UL << 1)    // HSI clock ready flag

/* Clock Source Selection Masks */
#define RCC_CFGR_SW_HSI     (0UL << 0)    // HSI as system clock
#define RCC_CFGR_SWS_HSI    (0UL << 2)    // HSI used as system clock
#define RCC_CFGR_SW_MASK    (3UL << 0)    // System clock switch mask
#define RCC_CFGR_SWS_MASK   (3UL << 2)    // System clock switch status mask

/* MCO1 Configuration Bits */
#define RCC_CFGR_MCO1_HSI   (0UL << 21)   // HSI clock selected as MCO1 source
#define RCC_CFGR_MCO1_MASK  (3UL << 21)   // MCO1 source selection mask
#define RCC_CFGR_MCO1PRE    (7UL << 24)   // MCO1 prescaler mask

/* Function Prototypes */
void HSI_Init(void);
void MCO1_OutputHSI(void);
#endif // HSI_H
