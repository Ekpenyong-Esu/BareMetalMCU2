/**
 ******************************************************************************
 * @file    accel_mma8452q.h
 * @brief   MMA8452Q register map and bit definitions
 ******************************************************************************
 */

#ifndef ACCEL_MMA8452Q_H
#define ACCEL_MMA8452Q_H

#ifdef __cplusplus
extern "C" {
#endif

/* Register addresses --------------------------------------------------------*/
#define ACCEL_REG_STATUS 0x00U
#define ACCEL_REG_OUT_X_MSB 0x01U
#define ACCEL_REG_OUT_X_LSB 0x02U
#define ACCEL_REG_OUT_Y_MSB 0x03U
#define ACCEL_REG_OUT_Y_LSB 0x04U
#define ACCEL_REG_OUT_Z_MSB 0x05U
#define ACCEL_REG_OUT_Z_LSB 0x06U
#define ACCEL_REG_F_SETUP 0x09U
#define ACCEL_REG_TRIG_CFG 0x0AU
#define ACCEL_REG_SYSMOD 0x0BU
#define ACCEL_REG_INT_SOURCE 0x0CU
#define ACCEL_REG_WHO_AM_I 0x0DU
#define ACCEL_REG_XYZ_DATA_CFG 0x0EU
#define ACCEL_REG_HP_FILTER_CUTOFF 0x0FU
#define ACCEL_REG_PL_STATUS 0x10U
#define ACCEL_REG_PL_CFG 0x11U
#define ACCEL_REG_PL_COUNT 0x12U
#define ACCEL_REG_PL_BF_ZCOMP 0x13U
#define ACCEL_REG_P_L_THS_REG 0x14U
#define ACCEL_REG_FF_MT_CFG 0x15U
#define ACCEL_REG_FF_MT_SRC 0x16U
#define ACCEL_REG_FF_MT_THS 0x17U
#define ACCEL_REG_FF_MT_COUNT 0x18U
#define ACCEL_REG_TRANSIENT_CFG 0x1DU
#define ACCEL_REG_TRANSIENT_SRC 0x1EU
#define ACCEL_REG_TRANSIENT_THS 0x1FU
#define ACCEL_REG_TRANSIENT_COUNT 0x20U
#define ACCEL_REG_PULSE_CFG 0x21U
#define ACCEL_REG_PULSE_SRC 0x22U
#define ACCEL_REG_PULSE_THSX 0x23U
#define ACCEL_REG_PULSE_THSY 0x24U
#define ACCEL_REG_PULSE_THSZ 0x25U
#define ACCEL_REG_PULSE_TMLT 0x26U
#define ACCEL_REG_PULSE_LTCY 0x27U
#define ACCEL_REG_PULSE_WIND 0x28U
#define ACCEL_REG_ASLP_COUNT 0x29U
#define ACCEL_REG_CTRL_REG1 0x2AU
#define ACCEL_REG_CTRL_REG2 0x2BU
#define ACCEL_REG_CTRL_REG3 0x2CU
#define ACCEL_REG_CTRL_REG4 0x2DU
#define ACCEL_REG_CTRL_REG5 0x2EU
#define ACCEL_REG_OFF_X 0x2FU
#define ACCEL_REG_OFF_Y 0x30U
#define ACCEL_REG_OFF_Z 0x31U

/* Device identity and transport ---------------------------------------------*/
#define ACCEL_DEVICE_ID 0x2AU     /**< WHO_AM_I response */
#define ACCEL_I2C_ADDRESS 0x1DU   /**< 7-bit I2C address */
#define ACCEL_SPI_READ_CMD 0x80U  /**< SPI read command prefix */
#define ACCEL_SPI_WRITE_CMD 0x00U /**< SPI write command prefix */

/* CTRL_REG1 -----------------------------------------------------------------*/
#define ACCEL_CTRL_REG1_ACTIVE 0x01U
#define ACCEL_CTRL_REG1_F_READ 0x02U
#define ACCEL_CTRL_REG1_LNOISE 0x04U
#define ACCEL_CTRL_REG1_ODR_MASK 0x38U
#define ACCEL_CTRL_REG1_ODR_SHIFT 3U

/* CTRL_REG2: bit 7 is self-test, bit 6 is software reset. */
#define ACCEL_CTRL_REG2_SELF_TEST 0x80U
#define ACCEL_CTRL_REG2_RESET 0x40U

/* CTRL_REG4 (enable) and CTRL_REG5 (route to INT1) share this bit layout. */
#define ACCEL_INT_BIT_DRDY 0x01U
#define ACCEL_INT_BIT_FF_MT 0x04U /**< Freefall and motion share one source */
#define ACCEL_INT_BIT_PULSE 0x08U

/* Misc ----------------------------------------------------------------------*/
#define ACCEL_XYZ_DATA_CFG_FS_MASK 0x03U
#define ACCEL_HP_FILTER_ENABLE 0x10U
#define ACCEL_SYSMOD_MASK 0x03U
#define ACCEL_SIGN_BIT_14 0x2000
#define ACCEL_SIGN_EXTEND_14 0xC000

#ifdef __cplusplus
}
#endif

#endif /* ACCEL_MMA8452Q_H */
