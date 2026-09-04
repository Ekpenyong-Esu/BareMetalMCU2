/**
 ******************************************************************************
 * @file    mems_l3gd20.h
 * @brief   L3GD20 register map and bit definitions
 ******************************************************************************
 */

#ifndef MEMS_L3GD20_H
#define MEMS_L3GD20_H

#ifdef __cplusplus
extern "C" {
#endif

/* Identity ------------------------------------------------------------------*/
#define L3GD20_WHO_AM_I_VALUE 0xD4U
#define L3GD20_DEVICE_NAME "L3GD20"

/* Register addresses --------------------------------------------------------*/
#define L3GD20_WHO_AM_I_ADDR 0x0FU
#define L3GD20_CTRL_REG1_ADDR 0x20U
#define L3GD20_CTRL_REG2_ADDR 0x21U
#define L3GD20_CTRL_REG3_ADDR 0x22U
#define L3GD20_CTRL_REG4_ADDR 0x23U
#define L3GD20_CTRL_REG5_ADDR 0x24U
#define L3GD20_REFERENCE_ADDR 0x25U
#define L3GD20_OUT_TEMP_ADDR 0x26U
#define L3GD20_STATUS_REG_ADDR 0x27U
#define L3GD20_OUT_X_L_ADDR 0x28U
#define L3GD20_OUT_X_H_ADDR 0x29U
#define L3GD20_OUT_Y_L_ADDR 0x2AU
#define L3GD20_OUT_Y_H_ADDR 0x2BU
#define L3GD20_OUT_Z_L_ADDR 0x2CU
#define L3GD20_OUT_Z_H_ADDR 0x2DU
#define L3GD20_FIFO_CTRL_REG_ADDR 0x2EU
#define L3GD20_FIFO_SRC_REG_ADDR 0x2FU
#define L3GD20_INT1_CFG_ADDR 0x30U
#define L3GD20_INT1_SRC_ADDR 0x31U
#define L3GD20_INT1_THS_XH_ADDR 0x32U
#define L3GD20_INT1_THS_XL_ADDR 0x33U
#define L3GD20_INT1_THS_YH_ADDR 0x34U
#define L3GD20_INT1_THS_YL_ADDR 0x35U
#define L3GD20_INT1_THS_ZH_ADDR 0x36U
#define L3GD20_INT1_THS_ZL_ADDR 0x37U
#define L3GD20_INT1_DURATION_ADDR 0x38U

/* CTRL_REG1 -----------------------------------------------------------------*/
#define L3GD20_POWER_DOWN 0x00U
#define L3GD20_NORMAL_MODE 0x08U
#define L3GD20_ODR_95Hz 0x00U
#define L3GD20_ODR_190Hz 0x40U
#define L3GD20_ODR_380Hz 0x80U
#define L3GD20_ODR_760Hz 0xC0U
#define L3GD20_BANDWIDTH_1 0x00U
#define L3GD20_BANDWIDTH_2 0x10U
#define L3GD20_BANDWIDTH_3 0x20U
#define L3GD20_BANDWIDTH_4 0x30U
#define L3GD20_AXIS_X_ENABLE 0x01U
#define L3GD20_AXIS_Y_ENABLE 0x02U
#define L3GD20_AXIS_Z_ENABLE 0x04U
#define L3GD20_AXES_ENABLE 0x07U

/* CTRL_REG3 (interrupt routing) ---------------------------------------------*/
#define L3GD20_CTRL_REG3_I1_INT1 0x80U
#define L3GD20_CTRL_REG3_I1_BOOT 0x40U
#define L3GD20_CTRL_REG3_H_LACTIVE 0x20U
#define L3GD20_CTRL_REG3_PP_OD 0x10U
#define L3GD20_CTRL_REG3_I2_DRDY 0x08U
#define L3GD20_CTRL_REG3_I2_WTM 0x04U
#define L3GD20_CTRL_REG3_I2_ORUN 0x02U
#define L3GD20_CTRL_REG3_I2_EMPTY 0x01U

/* CTRL_REG4 -----------------------------------------------------------------*/
#define L3GD20_FULLSCALE_250 0x00U
#define L3GD20_FULLSCALE_500 0x10U
#define L3GD20_FULLSCALE_2000 0x20U
#define L3GD20_BLE_LSB 0x00U
#define L3GD20_BLE_MSB 0x40U
#define L3GD20_CTRL_REG4_SELF_TEST 0x02U

/* Reset defaults ------------------------------------------------------------*/
#define L3GD20_CTRL_REG1_RESET 0x07U
#define L3GD20_CTRL_REG_RESET 0x00U

/* Sensitivity (mdps/LSB) ----------------------------------------------------*/
#define L3GD20_SENSITIVITY_250DPS 8.75f
#define L3GD20_SENSITIVITY_500DPS 17.50f
#define L3GD20_SENSITIVITY_2000DPS 70.0f

/* SPI framing ---------------------------------------------------------------*/
#define L3GD20_READ_CMD 0x80U
#define L3GD20_MULTIPLEBYTE_CMD 0x40U

#ifdef __cplusplus
}
#endif

#endif /* MEMS_L3GD20_H */
