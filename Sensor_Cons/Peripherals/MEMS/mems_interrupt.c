/**
  ******************************************************************************
  * @file    mems_interrupt.c
  * @brief   L3GD20 interrupt routing (CTRL_REG3)
  ******************************************************************************
  */

#include "mems_interrupt.h"
#include "mems_io.h"
#include "mems_l3gd20.h"

/* Each CTRL_REG3 bit is driven by one flag of the config struct. */
static const struct {
    size_t flagOffset;
    uint8_t bit;
} s_ctrlReg3Bits[] = {
    { offsetof(MEMS_InterruptConfigTypeDef, InterruptEnable),  L3GD20_CTRL_REG3_I1_INT1   },
    { offsetof(MEMS_InterruptConfigTypeDef, BootStatusEnable), L3GD20_CTRL_REG3_I1_BOOT   },
    { offsetof(MEMS_InterruptConfigTypeDef, ActiveLevel),      L3GD20_CTRL_REG3_H_LACTIVE },
    { offsetof(MEMS_InterruptConfigTypeDef, OutputType),       L3GD20_CTRL_REG3_PP_OD     },
    { offsetof(MEMS_InterruptConfigTypeDef, DataReadyEnable),  L3GD20_CTRL_REG3_I2_DRDY   },
    { offsetof(MEMS_InterruptConfigTypeDef, WatermarkEnable),  L3GD20_CTRL_REG3_I2_WTM    },
    { offsetof(MEMS_InterruptConfigTypeDef, OverrunEnable),    L3GD20_CTRL_REG3_I2_ORUN   },
    { offsetof(MEMS_InterruptConfigTypeDef, EmptyEnable),      L3GD20_CTRL_REG3_I2_EMPTY  },
};

#define MEMS_CTRL_REG3_BIT_COUNT (sizeof(s_ctrlReg3Bits) / sizeof(s_ctrlReg3Bits[0]))

static uint8_t MEMS_BuildCtrlReg3(const MEMS_InterruptConfigTypeDef *config)
{
    uint8_t reg = 0U;

    for (size_t i = 0U; i < MEMS_CTRL_REG3_BIT_COUNT; i++) {
        const bool *flag = (const bool *)((const uint8_t *)config + s_ctrlReg3Bits[i].flagOffset);
        if (*flag) {
            reg |= s_ctrlReg3Bits[i].bit;
        }
    }

    return reg;
}

MEMS_StatusTypeDef MEMS_ConfigureInterrupt(MEMS_HandleTypeDef *hmems,
                                           const MEMS_InterruptConfigTypeDef *config)
{
    MEMS_StatusTypeDef status = MEMS_CheckReady(hmems);

    if (status != MEMS_OK) {
        return status;
    }
    if (config == NULL) {
        return MEMS_INVALID_PARAM;
    }

    return MEMS_WriteRegister(hmems, L3GD20_CTRL_REG3_ADDR, MEMS_BuildCtrlReg3(config));
}
