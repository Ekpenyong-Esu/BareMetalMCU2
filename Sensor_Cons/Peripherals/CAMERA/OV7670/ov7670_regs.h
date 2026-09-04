/**
 ******************************************************************************
 * @file    ov7670_regs.h
 * @brief   OV7670 register map and bit definitions
 ******************************************************************************
 */

#ifndef OV7670_REGS_H
#define OV7670_REGS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Identity ------------------------------------------------------------------*/
#define OV7670_I2C_ADDRESS 0x42U /**< 8-bit write address (7-bit 0x21) */
#define OV7670_PID_VALUE 0x76U   /**< PID register, identifies the part */
#define OV7670_VER_VALUE 0x73U   /**< VER register, silicon revision */
#define OV7670_ID 0x7673U        /**< PID:VER of the shipping part */

/* Register addresses --------------------------------------------------------*/
#define OV7670_REG_GAIN 0x00U
#define OV7670_REG_BLUE 0x01U
#define OV7670_REG_RED 0x02U
#define OV7670_REG_VREF 0x03U
#define OV7670_REG_COM1 0x04U
#define OV7670_REG_BAVE 0x05U
#define OV7670_REG_GbAVE 0x06U
#define OV7670_REG_AECHH 0x07U
#define OV7670_REG_RAVE 0x08U
#define OV7670_REG_COM2 0x09U
#define OV7670_REG_PID 0x0AU
#define OV7670_REG_VER 0x0BU
#define OV7670_REG_COM3 0x0CU
#define OV7670_REG_COM4 0x0DU
#define OV7670_REG_COM5 0x0EU
#define OV7670_REG_COM6 0x0FU
#define OV7670_REG_AECH 0x10U
#define OV7670_REG_CLKRC 0x11U
#define OV7670_REG_COM7 0x12U
#define OV7670_REG_COM8 0x13U
#define OV7670_REG_COM9 0x14U
#define OV7670_REG_COM10 0x15U
#define OV7670_REG_HSTART 0x17U
#define OV7670_REG_HSTOP 0x18U
#define OV7670_REG_VSTART 0x19U
#define OV7670_REG_VSTOP 0x1AU
#define OV7670_REG_PSHFT 0x1BU
#define OV7670_REG_MIDH 0x1CU
#define OV7670_REG_MIDL 0x1DU
#define OV7670_REG_MVFP 0x1EU
#define OV7670_REG_LAEC 0x1FU
#define OV7670_REG_ADCCTR0 0x20U
#define OV7670_REG_ADCCTR1 0x21U
#define OV7670_REG_ADCCTR2 0x22U
#define OV7670_REG_ADCCTR3 0x23U
#define OV7670_REG_AEW 0x24U
#define OV7670_REG_AEB 0x25U
#define OV7670_REG_VPT 0x26U
#define OV7670_REG_BBIAS 0x27U
#define OV7670_REG_GbBIAS 0x28U
#define OV7670_REG_EXHCH 0x2AU
#define OV7670_REG_EXHCL 0x2BU
#define OV7670_REG_RBIAS 0x2CU
#define OV7670_REG_ADVFL 0x2DU
#define OV7670_REG_ADVFH 0x2EU
#define OV7670_REG_YAVE 0x2FU
#define OV7670_REG_HSYST 0x30U
#define OV7670_REG_HSYEN 0x31U
#define OV7670_REG_HREF 0x32U
#define OV7670_REG_CHLF 0x33U
#define OV7670_REG_ARBLM 0x34U
#define OV7670_REG_ADC 0x37U
#define OV7670_REG_ACOM 0x38U
#define OV7670_REG_OFON 0x39U
#define OV7670_REG_TSLB 0x3AU
#define OV7670_REG_COM11 0x3BU
#define OV7670_REG_COM12 0x3CU
#define OV7670_REG_COM13 0x3DU
#define OV7670_REG_COM14 0x3EU
#define OV7670_REG_EDGE 0x3FU
#define OV7670_REG_COM15 0x40U
#define OV7670_REG_COM16 0x41U
#define OV7670_REG_COM17 0x42U
#define OV7670_REG_AWBC1 0x43U
#define OV7670_REG_AWBC2 0x44U
#define OV7670_REG_AWBC3 0x45U
#define OV7670_REG_AWBC4 0x46U
#define OV7670_REG_AWBC5 0x47U
#define OV7670_REG_AWBC6 0x48U
#define OV7670_REG_REG4B 0x4BU
#define OV7670_REG_DNSTH 0x4CU
#define OV7670_REG_MTX1 0x4FU
#define OV7670_REG_MTX2 0x50U
#define OV7670_REG_MTX3 0x51U
#define OV7670_REG_MTX4 0x52U
#define OV7670_REG_MTX5 0x53U
#define OV7670_REG_MTX6 0x54U
#define OV7670_REG_BRIGHT 0x55U
#define OV7670_REG_CONTRAS 0x56U
#define OV7670_REG_CONTRAS_CENTER 0x57U
#define OV7670_REG_MTXS 0x58U
#define OV7670_REG_LCC1 0x62U
#define OV7670_REG_LCC2 0x63U
#define OV7670_REG_LCC3 0x64U
#define OV7670_REG_LCC4 0x65U
#define OV7670_REG_LCC5 0x66U
#define OV7670_REG_MANU 0x67U
#define OV7670_REG_MANV 0x68U
#define OV7670_REG_GFIX 0x69U
#define OV7670_REG_GGAIN 0x6AU
#define OV7670_REG_DBLV 0x6BU
#define OV7670_REG_AWBCTR3 0x6CU
#define OV7670_REG_AWBCTR2 0x6DU
#define OV7670_REG_AWBCTR1 0x6EU
#define OV7670_REG_AWBCTR0 0x6FU
#define OV7670_REG_SCALING_XSC 0x70U
#define OV7670_REG_SCALING_YSC 0x71U
#define OV7670_REG_SCALING_DCWCTR 0x72U
#define OV7670_REG_SCALING_PCLK_DIV 0x73U
#define OV7670_REG_REG74 0x74U
#define OV7670_REG_REG75 0x75U
#define OV7670_REG_REG76 0x76U
#define OV7670_REG_REG77 0x77U
#define OV7670_REG_DBLC1 0x87U
#define OV7670_REG_AECHG 0x8DU

/* COM2 ----------------------------------------------------------------------*/
#define OV7670_COM2_SOFT_SLEEP 0x10U

/* COM7 ----------------------------------------------------------------------*/
#define OV7670_COM7_RESET 0x80U
#define OV7670_COM7_FMT_QVGA 0x10U
#define OV7670_COM7_FMT_QCIF 0x08U
#define OV7670_COM7_FMT_RGB 0x04U
#define OV7670_COM7_COLOR_BAR 0x02U
#define OV7670_COM7_FMT_RAW 0x01U
/* Output format is the {RGB, RAW} bit pair; clear both before selecting. */
#define OV7670_COM7_FMT_MASK (OV7670_COM7_FMT_RGB | OV7670_COM7_FMT_RAW)
#define OV7670_COM7_FMT_YUV 0x00U
#define OV7670_COM7_FMT_VGA 0x00U

/* COM11 ---------------------------------------------------------------------*/
#define OV7670_COM11_NIGHT_MODE 0x80U

/* COM15 (companion of the COM7 format bits) ---------------------------------*/
#define OV7670_COM15_RGB565 0xC0U
#define OV7670_COM15_RGB555 0xD0U
#define OV7670_COM15_YUV422 0x00U
#define OV7670_COM15_RAW_BAYER 0x10U

/* MVFP ----------------------------------------------------------------------*/
#define OV7670_MVFP_MIRROR 0x20U
#define OV7670_MVFP_VFLIP 0x10U

/* SCALING_XSC test-pattern bits ---------------------------------------------*/
#define OV7670_XSC_TEST_PATTERN_1 0x02U
#define OV7670_XSC_TEST_PATTERN_2 0x08U
#define OV7670_XSC_TEST_PATTERN_BARS 0x0AU
#define OV7670_XSC_TEST_PATTERN_MASK 0x0AU

/* End marker for register tables --------------------------------------------*/
#define OV7670_REG_LIST_END 0xFFU

#ifdef __cplusplus
}
#endif

#endif /* OV7670_REGS_H */
