/**
 * @file ts_stmpe811.h
 * @brief STMPE811 register map and bit definitions
 * @details Device vocabulary only. Nothing here depends on how the driver is
 *          structured, so every touchscreen module can include it freely.
 */

#ifndef TS_STMPE811_H
#define TS_STMPE811_H

#ifdef __cplusplus
extern "C" {
#endif

/* Device identity -----------------------------------------------------------*/
#define STMPE811_I2C_ADDRESS 0x82 /*!< 8-bit I2C address (7-bit << 1), matches HAL DevAddress */
#define STMPE811_CHIP_ID 0x0811
#define STMPE811_MAX_X 4095
#define STMPE811_MAX_Y 4095

/* Register addresses --------------------------------------------------------*/
#define STMPE811_REG_CHIP_ID 0x00
#define STMPE811_REG_ID_VER 0x02
#define STMPE811_REG_SYS_CTRL1 0x03
#define STMPE811_REG_SYS_CTRL2 0x04
#define STMPE811_REG_SPI_CFG 0x08
#define STMPE811_REG_INT_CTRL 0x09
#define STMPE811_REG_INT_EN 0x0A
#define STMPE811_REG_INT_STA 0x0B
#define STMPE811_REG_GPIO_EN 0x0C
#define STMPE811_REG_GPIO_INT_STA 0x0D
#define STMPE811_REG_IO_AF 0x17
#define STMPE811_REG_ADC_CTRL1 0x20
#define STMPE811_REG_ADC_CTRL2 0x21
#define STMPE811_REG_ADC_CAPT 0x22
#define STMPE811_REG_ADC_DATA_CH0 0x30
#define STMPE811_REG_ADC_DATA_CH1 0x32
#define STMPE811_REG_ADC_DATA_CH4 0x38
#define STMPE811_REG_ADC_DATA_CH5 0x3A
#define STMPE811_REG_ADC_DATA_CH6 0x3C
#define STMPE811_REG_ADC_DATA_CH7 0x3E
#define STMPE811_REG_TSC_CTRL 0x40
#define STMPE811_REG_TSC_CFG 0x41
#define STMPE811_REG_WDW_TR_X 0x42
#define STMPE811_REG_WDW_TR_Y 0x44
#define STMPE811_REG_WDW_BL_X 0x46
#define STMPE811_REG_WDW_BL_Y 0x48
#define STMPE811_REG_FIFO_TH 0x4A
#define STMPE811_REG_FIFO_STA 0x4B
#define STMPE811_REG_FIFO_SIZE 0x4C
#define STMPE811_REG_TSC_DATA_X 0x4D
#define STMPE811_REG_TSC_DATA_Y 0x4F
#define STMPE811_REG_TSC_DATA_Z 0x51
#define STMPE811_REG_TSC_DATA_XYZ 0x52
#define STMPE811_REG_TSC_FRACT_XYZ 0x56
#define STMPE811_REG_TSC_DATA 0x57
#define STMPE811_REG_TSC_I_DRIVE 0x58
#define STMPE811_REG_TSC_SHIELD 0x59
#define STMPE811_REG_TSC_DATA_NON_INC 0xD7

/* IO_AF pin masks -----------------------------------------------------------*/
#define STMPE811_PIN_4 0x10
#define STMPE811_PIN_5 0x20
#define STMPE811_PIN_6 0x40
#define STMPE811_PIN_7 0x80
#define STMPE811_TOUCH_IO_ALL (STMPE811_PIN_4 | STMPE811_PIN_5 | STMPE811_PIN_6 | STMPE811_PIN_7)

/* SYS_CTRL bits -------------------------------------------------------------*/
#define STMPE811_SYS_CTRL1_HIBERNATE 0x01
#define STMPE811_SYS_CTRL1_RESET 0x02
#define STMPE811_SYS_CTRL2_ADC_OFF 0x01
#define STMPE811_SYS_CTRL2_TSC_OFF 0x02
#define STMPE811_SYS_CTRL2_GPIO_OFF 0x04
#define STMPE811_SYS_CTRL2_TS_OFF 0x08

/* INT_CTRL bits -------------------------------------------------------------*/
#define STMPE811_INT_CTRL_POL_HIGH 0x04
#define STMPE811_INT_CTRL_POL_LOW 0x00
#define STMPE811_INT_CTRL_EDGE 0x02
#define STMPE811_INT_CTRL_LEVEL 0x00
#define STMPE811_INT_CTRL_ENABLE 0x01
#define STMPE811_INT_CTRL_DISABLE 0x00

/* INT_EN / INT_STA bits -----------------------------------------------------*/
#define STMPE811_INT_EN_TOUCH_DET 0x01
#define STMPE811_INT_EN_FIFO_TH 0x02
#define STMPE811_INT_EN_FIFO_OFLOW 0x04
#define STMPE811_INT_EN_FIFO_FULL 0x08
#define STMPE811_INT_EN_FIFO_EMPTY 0x10
#define STMPE811_INT_EN_TEMP_SENS 0x20
#define STMPE811_INT_EN_ADC 0x40
#define STMPE811_INT_EN_GPIO 0x80
#define STMPE811_INT_CLEAR_ALL 0xFF /*!< Write-1-to-clear every interrupt flag */

/* TSC_CTRL bits -------------------------------------------------------------*/
#define STMPE811_TSC_CTRL_EN 0x01
#define STMPE811_TSC_CTRL_XYZ 0x00
#define STMPE811_TSC_CTRL_XY 0x02
#define STMPE811_TS_CTRL_STATUS 0x80

/* TSC_CFG fields ------------------------------------------------------------*/
#define STMPE811_TSC_CFG_1_SAMPLE 0x00
#define STMPE811_TSC_CFG_2_SAMPLE 0x40
#define STMPE811_TSC_CFG_4_SAMPLE 0x80
#define STMPE811_TSC_CFG_8_SAMPLE 0xC0
#define STMPE811_TSC_CFG_DELAY_10US 0x00
#define STMPE811_TSC_CFG_DELAY_50US 0x08
#define STMPE811_TSC_CFG_DELAY_100US 0x10
#define STMPE811_TSC_CFG_DELAY_500US 0x18
#define STMPE811_TSC_CFG_DELAY_1MS 0x20
#define STMPE811_TSC_CFG_DELAY_5MS 0x28
#define STMPE811_TSC_CFG_DELAY_10MS 0x30
#define STMPE811_TSC_CFG_DELAY_50MS 0x38
#define STMPE811_TSC_CFG_SETTLE_10US 0x00
#define STMPE811_TSC_CFG_SETTLE_100US 0x01
#define STMPE811_TSC_CFG_SETTLE_500US 0x02
#define STMPE811_TSC_CFG_SETTLE_1MS 0x03
#define STMPE811_TSC_CFG_SETTLE_5MS 0x04
#define STMPE811_TSC_CFG_SETTLE_10MS 0x05
#define STMPE811_TSC_CFG_SETTLE_50MS 0x06
#define STMPE811_TSC_CFG_SETTLE_100MS 0x07

/* FIFO_STA values -----------------------------------------------------------*/
#define STMPE811_FIFO_RESET 0x01       /*!< Hold the FIFO in reset */
#define STMPE811_FIFO_OPERATIONAL 0x00 /*!< Release the FIFO back into operation */

#ifdef __cplusplus
}
#endif

#endif /* TS_STMPE811_H */
