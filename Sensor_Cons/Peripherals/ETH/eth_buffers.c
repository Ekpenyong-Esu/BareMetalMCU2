/**
 * @file eth_buffers.c
 * @brief Internal DMA descriptor and packet buffer ownership
 */

#include "eth_buffers.h"
#include "log.h"

/*
 * The descriptor rings and the receive buffers are read and written by the
 * Ethernet DMA, so they must live in AHB accessible RAM. Leaving them in the
 * default .bss places them in SRAM1/2/3 at 0x20000000, which the MAC can
 * reach. They must not be moved into CCM RAM, which is core only.
 */
static ETH_DMADescTypeDef s_rxDescTab[ETH_RX_DESC_CNT] __attribute__((aligned(4)));
static ETH_DMADescTypeDef s_txDescTab[ETH_TX_DESC_CNT] __attribute__((aligned(4)));

/* One receive buffer per descriptor, plus a single transmit staging buffer. */
static uint8_t s_rxPool[ETH_RX_DESC_CNT][ETH_BUFFER_SIZE] __attribute__((aligned(4)));
static uint8_t s_txBuffer[ETH_BUFFER_SIZE] __attribute__((aligned(4)));

static uint32_t s_rxAllocIdx = 0U;

void ETH_Buffers_Attach(ETH_HandleTypeDef *heth) {
    if (heth == NULL) {
        return;
    }

    heth->Init.TxDesc = s_txDescTab;
    heth->Init.RxDesc = s_rxDescTab;
    heth->Init.RxBuffLen = ETH_BUFFER_SIZE;
}

void ETH_Buffers_Reset(void) {
    s_rxAllocIdx = 0U;
}

uint8_t *ETH_Buffers_GetTxBuffer(void) {
    return s_txBuffer;
}

/**
 * @brief   Hand a free buffer to the receive DMA
 * @note    Overrides the HAL weak default, which leaves the pointer untouched
 *          and would make the DMA write to whatever address the descriptor
 *          happened to hold.
 */
void HAL_ETH_RxAllocateCallback(uint8_t **buff) {
    *buff = s_rxPool[s_rxAllocIdx];
    s_rxAllocIdx = (s_rxAllocIdx + 1U) % ETH_RX_DESC_CNT;
}

/**
 * @brief   Chain the buffers that make up one received packet
 * @note    ETH_BUFFER_SIZE exceeds the longest legal frame, so a packet always
 *          occupies exactly one buffer and the list has a single element.
 */
void HAL_ETH_RxLinkCallback(void **pStart, void **pEnd, uint8_t *buff, uint16_t Length) {
    (void)Length;

    if (*pStart == NULL) {
        *pStart = buff;
    }
    *pEnd = buff;
}

/**
 * @brief   Release a transmitted buffer
 * @note    The transmit buffer is statically owned by this module, so there is
 *          nothing to free. The override exists so the HAL does not fall back
 *          to its empty weak default silently.
 */
/* The prototype is fixed by stm32f4xx_hal_eth.h, so the pointer cannot be const. */
void HAL_ETH_TxFreeCallback(uint32_t *buff) { // NOLINT(readability-non-const-parameter)
    (void)buff;
}
