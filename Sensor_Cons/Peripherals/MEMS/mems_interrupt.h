/**
 ******************************************************************************
 * @file    mems_interrupt.h
 * @brief   L3GD20 interrupt routing (CTRL_REG3)
 ******************************************************************************
 */

#ifndef MEMS_INTERRUPT_H
#define MEMS_INTERRUPT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mems_types.h"

MEMS_StatusTypeDef MEMS_ConfigureInterrupt(MEMS_HandleTypeDef *hmems,
                                           const MEMS_InterruptConfigTypeDef *config);

#ifdef __cplusplus
}
#endif

#endif /* MEMS_INTERRUPT_H */
