/**
 ******************************************************************************
 * @file    mcp2515_regs.h
 * @brief   MCP2515 SPI instructions, register map and bit positions
 ******************************************************************************
 */

#ifndef MCP2515_REGS_H
#define MCP2515_REGS_H

#ifdef __cplusplus
extern "C" {
#endif

/* SPI instructions ----------------------------------------------------------*/

#define MCP2515_CMD_RESET 0xC0U
#define MCP2515_CMD_READ 0x03U
#define MCP2515_CMD_WRITE 0x02U
#define MCP2515_CMD_RTS_TXB0 0x81U /*!< Request-to-send for TX buffer 0 */
#define MCP2515_CMD_READ_STATUS 0xA0U
#define MCP2515_CMD_BIT_MODIFY 0x05U

/* Registers -----------------------------------------------------------------*/

#define MCP2515_REG_CANSTAT 0x0EU
#define MCP2515_REG_CANCTRL 0x0FU

#define MCP2515_REG_CNF3 0x28U
#define MCP2515_REG_CNF2 0x29U
#define MCP2515_REG_CNF1 0x2AU
#define MCP2515_REG_CANINTE 0x2BU
#define MCP2515_REG_CANINTF 0x2CU

#define MCP2515_REG_TXB0CTRL 0x30U
#define MCP2515_REG_TXB0SIDH 0x31U

#define MCP2515_REG_RXB0CTRL 0x60U
#define MCP2515_REG_RXB0SIDH 0x61U
#define MCP2515_REG_RXB1CTRL 0x70U
#define MCP2515_REG_RXB1SIDH 0x71U

/* Bits ----------------------------------------------------------------------*/

#define MCP2515_CANCTRL_REQOP_MASK 0xE0U /*!< Requested operating mode */
#define MCP2515_CANSTAT_OPMOD_MASK 0xE0U /*!< Mode actually entered */

#define MCP2515_CNF2_BTLMODE 0x80U /*!< PS2 length comes from CNF3, not PS1 */

#define MCP2515_TXBCTRL_TXREQ 0x08U /*!< Set while a transmission is queued */

#define MCP2515_CANINTF_RX0IF 0x01U
#define MCP2515_CANINTF_RX1IF 0x02U

#define MCP2515_RXBCTRL_RXM_ANY 0x60U /*!< Masks and filters off: accept everything */
#define MCP2515_RXB0CTRL_BUKT 0x04U   /*!< Overflow from RXB0 rolls into RXB1 */

/* Frame field encoding ------------------------------------------------------*/

#define MCP2515_SIDL_EXIDE 0x08U /*!< Set in TX to send an extended id */
#define MCP2515_SIDL_IDE 0x08U   /*!< Set in RX when the id received was extended */
#define MCP2515_SIDL_SRR 0x10U   /*!< Remote request, standard frames */
#define MCP2515_DLC_RTR 0x40U    /*!< Remote request, extended frames */
#define MCP2515_DLC_MASK 0x0FU

/* Extended (29-bit) id layout across SIDH, SIDL, EID8 and EID0:
   SIDH holds id[28:21], SIDL[7:5] holds id[20:18], SIDL[1:0] holds id[17:16],
   EID8 holds id[15:8] and EID0 holds id[7:0]. */
#define MCP2515_EXT_ID_SIDH_SHIFT 21U       /*!< id bits carried by SIDH */
#define MCP2515_EXT_ID_SIDL_HIGH_SHIFT 13U  /*!< id[20:18] into SIDL[7:5] */
#define MCP2515_EXT_ID_SIDL_HIGH_MASK 0xE0U /*!< SIDL[7:5] */
#define MCP2515_EXT_ID_SIDL_LOW_SHIFT 16U   /*!< id[17:16] into SIDL[1:0] */
#define MCP2515_EXT_ID_EID8_SHIFT 8U        /*!< id bits carried by EID8 */

#ifdef __cplusplus
}
#endif

#endif /* MCP2515_REGS_H */
