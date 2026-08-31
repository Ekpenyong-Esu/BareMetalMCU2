/**
  ******************************************************************************
  * @file    ov7670_io.c
  * @brief   SCCB (I2C) register access for the OV7670
  ******************************************************************************
  */

#include "ov7670_io.h"
#include "ov7670_regs.h"
#include "i2c_transfer.h"

OV7670_StatusTypeDef OV7670_WriteReg(OV7670_Handle_t *hov7670, uint8_t reg, uint8_t value)
{
    if (hov7670 == NULL || !I2C_DeviceIsReady(&hov7670->device)) {
        return OV7670_INVALID_PARAM;
    }

    if (I2C_Mem_Write(&hov7670->device, reg, I2C_MEMADD_SIZE_8BIT,
                      &value, 1, OV7670_I2C_TIMEOUT) != I2C_OK) {
        return OV7670_I2C_ERROR;
    }

    return OV7670_OK;
}

OV7670_StatusTypeDef OV7670_ReadReg(OV7670_Handle_t *hov7670, uint8_t reg, uint8_t *value)
{
    if (hov7670 == NULL || !I2C_DeviceIsReady(&hov7670->device) || value == NULL) {
        return OV7670_INVALID_PARAM;
    }

    if (I2C_Mem_Read(&hov7670->device, reg, I2C_MEMADD_SIZE_8BIT,
                     value, 1, OV7670_I2C_TIMEOUT) != I2C_OK) {
        return OV7670_I2C_ERROR;
    }

    return OV7670_OK;
}

OV7670_StatusTypeDef OV7670_UpdateReg(OV7670_Handle_t *hov7670, uint8_t reg,
                                      uint8_t mask, uint8_t value)
{
    OV7670_StatusTypeDef status;
    uint8_t reg_val = 0U;

    status = OV7670_ReadReg(hov7670, reg, &reg_val);
    if (status != OV7670_OK) {
        return status;
    }

    reg_val = (uint8_t)((reg_val & (uint8_t)~mask) | (value & mask));

    return OV7670_WriteReg(hov7670, reg, reg_val);
}

OV7670_StatusTypeDef OV7670_WriteRegList(OV7670_Handle_t *hov7670, const OV7670_RegVal_t *reg_list)
{
    if (reg_list == NULL) {
        return OV7670_INVALID_PARAM;
    }

    while (reg_list->reg_addr != OV7670_REG_LIST_END) {
        OV7670_StatusTypeDef status = OV7670_WriteReg(hov7670, reg_list->reg_addr, reg_list->value);
        if (status != OV7670_OK) {
            return status;
        }
        reg_list++;
    }

    return OV7670_OK;
}
