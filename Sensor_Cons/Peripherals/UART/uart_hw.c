/**
 * @file uart_hw.c
 * @brief HAL MSP for the UART driver: clocks, pins, DMA streams and NVIC
 *
 * Everything here is derived from the config the application handed to Init():
 * the instance selects the peripheral clock, the IRQ vector and the default
 * alternate function; the pins and DMA streams are taken verbatim. The HAL
 * calls in with only its own handle, so the owning link is looked up through
 * the registry in uart.c, the same way can_core.c does for CAN.
 *
 * Exports no header: HAL_UART_MspInit()/MspDeInit() are declared by the HAL.
 */

#include "uart.h"
#include "gpio.h"
#include "log.h"

/* Private constants ---------------------------------------------------------*/

#define UART_IRQ_PRIORITY 0U
#define UART_DMA_IRQ_PRIORITY 2U

/* Instance-derived facts ----------------------------------------------------*/

/* RCC gates each UART with a different bit in a different register, and the
   HAL only offers them as macros, so the mapping is spelled out per instance. */
static void UART_HW_SetClock(const USART_TypeDef *instance, bool enable) {
    if (instance == USART1) {
        if (enable) {
            __HAL_RCC_USART1_CLK_ENABLE();
        }
        else {
            __HAL_RCC_USART1_CLK_DISABLE();
        }
    }
    else if (instance == USART2) {
        if (enable) {
            __HAL_RCC_USART2_CLK_ENABLE();
        }
        else {
            __HAL_RCC_USART2_CLK_DISABLE();
        }
    }
    else if (instance == USART3) {
        if (enable) {
            __HAL_RCC_USART3_CLK_ENABLE();
        }
        else {
            __HAL_RCC_USART3_CLK_DISABLE();
        }
    }
    else if (instance == UART4) {
        if (enable) {
            __HAL_RCC_UART4_CLK_ENABLE();
        }
        else {
            __HAL_RCC_UART4_CLK_DISABLE();
        }
    }
    else if (instance == UART5) {
        if (enable) {
            __HAL_RCC_UART5_CLK_ENABLE();
        }
        else {
            __HAL_RCC_UART5_CLK_DISABLE();
        }
    }
    else if (instance == USART6) {
        if (enable) {
            __HAL_RCC_USART6_CLK_ENABLE();
        }
        else {
            __HAL_RCC_USART6_CLK_DISABLE();
        }
    }
}

static const struct {
    const USART_TypeDef *instance;
    IRQn_Type irq;
} s_instanceIrqs[] = {
    {USART1, USART1_IRQn}, {USART2, USART2_IRQn}, {USART3, USART3_IRQn},
    {UART4, UART4_IRQn},   {UART5, UART5_IRQn},   {USART6, USART6_IRQn},
};

#define UART_INSTANCE_IRQ_COUNT (sizeof(s_instanceIrqs) / sizeof(s_instanceIrqs[0]))

static bool UART_HW_ResolveIrq(const USART_TypeDef *instance, IRQn_Type *irq) {
    for (size_t i = 0; i < UART_INSTANCE_IRQ_COUNT; i++) {
        if (s_instanceIrqs[i].instance == instance) {
            *irq = s_instanceIrqs[i].irq;
            return true;
        }
    }

    return false;
}

/* On STM32F4 the USARTs share AF7 and the UART4/5/USART6 group AF8, so the
   application only has to spell it out for an unusual pin. */
static uint8_t UART_HW_ResolveAlternate(const UART_Config_t *config) {
    if (config->alternate != 0U) {
        return config->alternate;
    }

    if (config->instance == USART1 || config->instance == USART2 || config->instance == USART3) {
        return GPIO_AF7_USART1;
    }

    return GPIO_AF8_UART4;
}

/* Stream-derived facts ------------------------------------------------------*/

/* The DMA2 vectors are not contiguous in the NVIC table (streams 5..7 sit
   after the other DMA2 entries), so a lookup table beats arithmetic. */
static const struct {
    const DMA_Stream_TypeDef *stream;
    IRQn_Type irq;
} s_dmaStreamIrqs[] = {
    {DMA1_Stream0, DMA1_Stream0_IRQn}, {DMA1_Stream1, DMA1_Stream1_IRQn},
    {DMA1_Stream2, DMA1_Stream2_IRQn}, {DMA1_Stream3, DMA1_Stream3_IRQn},
    {DMA1_Stream4, DMA1_Stream4_IRQn}, {DMA1_Stream5, DMA1_Stream5_IRQn},
    {DMA1_Stream6, DMA1_Stream6_IRQn}, {DMA1_Stream7, DMA1_Stream7_IRQn},
    {DMA2_Stream0, DMA2_Stream0_IRQn}, {DMA2_Stream1, DMA2_Stream1_IRQn},
    {DMA2_Stream2, DMA2_Stream2_IRQn}, {DMA2_Stream3, DMA2_Stream3_IRQn},
    {DMA2_Stream4, DMA2_Stream4_IRQn}, {DMA2_Stream5, DMA2_Stream5_IRQn},
    {DMA2_Stream6, DMA2_Stream6_IRQn}, {DMA2_Stream7, DMA2_Stream7_IRQn},
};

#define UART_DMA_STREAM_COUNT (sizeof(s_dmaStreamIrqs) / sizeof(s_dmaStreamIrqs[0]))

static bool UART_HW_ResolveDmaIrq(const DMA_Stream_TypeDef *stream, IRQn_Type *irq) {
    for (size_t i = 0; i < UART_DMA_STREAM_COUNT; i++) {
        if (s_dmaStreamIrqs[i].stream == stream) {
            *irq = s_dmaStreamIrqs[i].irq;
            return true;
        }
    }

    return false;
}

static void UART_HW_EnableDmaClock(const DMA_Stream_TypeDef *stream) {
    if ((uint32_t)stream >= DMA2_BASE) {
        __HAL_RCC_DMA2_CLK_ENABLE();
    }
    else {
        __HAL_RCC_DMA1_CLK_ENABLE();
    }
}

/* Pins ----------------------------------------------------------------------*/

static void UART_HW_InitPins(const UART_Config_t *config) {
    GPIO_InitTypeDef gpio = {0};

    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = UART_HW_ResolveAlternate(config);

    /* The GPIO driver owns the port clocks. */
    if (config->txPort != NULL) {
        gpio.Pin = config->txPin;
        if (GPIO_Driver_Pin_Init(config->txPort, &gpio) != HAL_OK) {
            log_error("UART: TX pin init failed");
        }
    }

    if (config->rxPort != NULL) {
        gpio.Pin = config->rxPin;
        if (GPIO_Driver_Pin_Init(config->rxPort, &gpio) != HAL_OK) {
            log_error("UART: RX pin init failed");
        }
    }
}

static void UART_HW_DeInitPins(const UART_Config_t *config) {
    if (config->txPort != NULL) {
        (void)GPIO_Driver_Pin_DeInit(config->txPort, config->txPin);
    }

    if (config->rxPort != NULL) {
        (void)GPIO_Driver_Pin_DeInit(config->rxPort, config->rxPin);
    }
}

/* DMA -----------------------------------------------------------------------*/

/* Returns false without linking, so UART_DMA_Init() sees a NULL hdmatx/hdmarx
   and refuses to open rather than dereferencing it on the first transfer. */
static bool UART_HW_InitDmaStream(DMA_HandleTypeDef *hdma, DMA_Stream_TypeDef *stream,
                                  uint32_t channel, uint32_t direction, uint32_t priority) {
    IRQn_Type irq = (IRQn_Type)0;

    if (!UART_HW_ResolveDmaIrq(stream, &irq)) {
        log_error("UART: unknown DMA stream");
        return false;
    }

    UART_HW_EnableDmaClock(stream);

    hdma->Instance = stream;
    hdma->Init.Channel = channel;
    hdma->Init.Direction = direction;
    hdma->Init.PeriphInc = DMA_PINC_DISABLE;
    hdma->Init.MemInc = DMA_MINC_ENABLE;
    hdma->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma->Init.Mode = DMA_NORMAL;
    hdma->Init.Priority = priority;
    hdma->Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    if (HAL_DMA_Init(hdma) != HAL_OK) {
        log_error("UART: DMA stream init failed");
        return false;
    }

    HAL_NVIC_SetPriority(irq, UART_DMA_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(irq);
    return true;
}

static void UART_HW_InitDma(UART_Handle_t *handle) {
    const UART_Config_t *config = &handle->config;

    if (UART_HW_InitDmaStream(&handle->hdmaTx, config->dmaTxStream, config->dmaTxChannel,
                              DMA_MEMORY_TO_PERIPH, DMA_PRIORITY_LOW)) {
        __HAL_LINKDMA(handle->huart, hdmatx, handle->hdmaTx);
    }

    if (UART_HW_InitDmaStream(&handle->hdmaRx, config->dmaRxStream, config->dmaRxChannel,
                              DMA_PERIPH_TO_MEMORY, DMA_PRIORITY_HIGH)) {
        __HAL_LINKDMA(handle->huart, hdmarx, handle->hdmaRx);
    }
}

static void UART_HW_DeInitDmaStream(DMA_HandleTypeDef *hdma) {
    IRQn_Type irq = (IRQn_Type)0;

    if (hdma == NULL) {
        return;
    }

    (void)HAL_DMA_DeInit(hdma);

    if (UART_HW_ResolveDmaIrq(hdma->Instance, &irq)) {
        HAL_NVIC_DisableIRQ(irq);
    }
}

/* HAL MSP -------------------------------------------------------------------*/

/**
 * @brief HAL MSP init: called by HAL_UART_Init() to bring up clock, pins, DMA, NVIC
 * @param huart HAL handle whose Instance selects the link in the registry
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart) {
    UART_Handle_t *handle = UART_FromInstance(huart->Instance);
    IRQn_Type irq = (IRQn_Type)0;

    if (handle == NULL) {
        log_error("UART: MspInit for an unregistered instance");
        return;
    }

    UART_HW_SetClock(huart->Instance, true);
    UART_HW_InitPins(&handle->config);

    if (handle->config.mode == UART_MODE_DMA) {
        UART_HW_InitDma(handle);
    }

    /* Blocking mode polls, so leaving its vector disarmed keeps a stray IDLE
       or error flag from invoking callbacks nobody is waiting on. */
    if (handle->config.mode != UART_MODE_BLOCKING && UART_HW_ResolveIrq(huart->Instance, &irq)) {
        HAL_NVIC_SetPriority(irq, UART_IRQ_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(irq);
    }
}

/**
 * @brief HAL MSP deinit: called by HAL_UART_DeInit() to release what MspInit took
 * @param huart HAL handle whose Instance selects the link in the registry
 */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart) {
    UART_Handle_t *handle = UART_FromInstance(huart->Instance);
    IRQn_Type irq = (IRQn_Type)0;

    UART_HW_SetClock(huart->Instance, false);

    if (UART_HW_ResolveIrq(huart->Instance, &irq)) {
        HAL_NVIC_DisableIRQ(irq);
    }

    UART_HW_DeInitDmaStream(huart->hdmatx);
    UART_HW_DeInitDmaStream(huart->hdmarx);
    huart->hdmatx = NULL;
    huart->hdmarx = NULL;

    if (handle != NULL) {
        UART_HW_DeInitPins(&handle->config);
    }
}
