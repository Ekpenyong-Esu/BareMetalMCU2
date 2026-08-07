/**
  ******************************************************************************
  * @file    ov7670_io.h
  * @brief   SCCB (I2C) register access for the OV7670
  ******************************************************************************
  */

#ifndef OV7670_IO_H
#define OV7670_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ov7670_types.h"

/**
 * @brief One entry of a register initialisation table.
 */
typedef struct {
    uint8_t reg_addr;
    uint8_t value;
} OV7670_RegVal_t;

/**
 * @note These run before OV7670_Init() completes, so they only require an I2C
 *       handle - not a fully initialised driver.
 */
OV7670_StatusTypeDef OV7670_WriteReg(OV7670_Handle_t *hov7670, uint8_t reg, uint8_t value);
OV7670_StatusTypeDef OV7670_ReadReg(OV7670_Handle_t *hov7670, uint8_t reg, uint8_t *value);

/**
 * @brief Read-modify-write the bits selected by @p mask.
 */
OV7670_StatusTypeDef OV7670_UpdateReg(OV7670_Handle_t *hov7670, uint8_t reg,
                                      uint8_t mask, uint8_t value);

/**
 * @brief Write entries until one with address OV7670_REG_LIST_END is reached.
 */
OV7670_StatusTypeDef OV7670_WriteRegList(OV7670_Handle_t *hov7670, const OV7670_RegVal_t *reg_list);

#ifdef __cplusplus
}
#endif

#endif /* OV7670_IO_H */
