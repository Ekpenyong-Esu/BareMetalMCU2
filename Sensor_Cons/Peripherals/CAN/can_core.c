/**
 ******************************************************************************
 * @file    can_core.c
 * @brief   CAN lifecycle and handle registry implementation
 *
 * This module implements the CAN core functionality:
 * - Initialization/deinitialization with clock, GPIO, bit-timing setup
 * - Handle registry for HAL callback routing (instance -> handle)
 * - Mode switching without full reinit
 * - Status queries and error bookkeeping
 *
 * Key Design Points:
 * - Handle registry (s_registry) maps CAN instance index -> handle
 * - Registry populated in CAN_Init() before HAL_CAN_Init() so MspInit
 *   can find the handle for clock/GPIO setup
 * - CAN_FromInstance() is the only reader; CAN_Init/DeInit are the only writers
 * - Bit timing is derived from live PCLK1 via CAN_ApplyBaudRate()
 * - Filter bank split is config-driven (varies single- vs dual-CAN parts)
 */

#include "can_core.h"
#include "can_baudrate.h"
#include "can_events.h"
#include "gpio.h"
#include <string.h>
#include "log.h"

/* Private constants ---------------------------------------------------------*/

/** @brief Default alternate function for bxCAN on STM32F4 (AF9 for both CAN1 and CAN2) */
#define CAN_DEFAULT_AF 9U
/** @brief Default filter bank count for STM32F429 single-CAN parts */
#define CAN_DEFAULT_FILTER_BANKS CAN_MAX_FILTER_BANKS

/* Private variables ---------------------------------------------------------*/

/** @brief Instance index to owning handle; the only file-scope state here */
static CAN_Handle_t *s_registry[2] = {NULL};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Map a CAN instance pointer to a registry index
 * @param instance CAN1 or CAN2
 * @retval int 0 for CAN1, 1 for CAN2, -1 for unknown
 */
static int CAN_InstanceIndex(const CAN_TypeDef *instance) {
    if (instance == (const CAN_TypeDef *)CAN1) {
        return 0;
    }
#ifdef CAN2
    if (instance == (const CAN_TypeDef *)CAN2) {
        return 1;
    }
#endif
    return -1;
}

/**
 * @brief Resolve the driver handle that owns a HAL instance
 * @param instance CAN1 or CAN2
 * @retval CAN_Handle_t* Owning handle, or NULL when unregistered
 */
CAN_Handle_t *CAN_FromInstance(const CAN_TypeDef *instance) {
    int index = CAN_InstanceIndex(instance);

    return (index < 0) ? NULL : s_registry[index];
}

/* Error mapping -------------------------------------------------------------*/

/* Ordered most severe first: HAL reports several flags at once and the first
   match wins, so a stuff error must not hide a bus-off condition. */
static const struct {
    uint32_t hal_flag;   /*!< HAL_CAN_ERROR_* flag */
    CAN_ErrorType error; /*!< Corresponding driver error */
} s_errorMap[] = {
    {HAL_CAN_ERROR_BOF, CAN_ERROR_BUS_OFF},      {HAL_CAN_ERROR_EPV, CAN_ERROR_BUS_PASSIVE},
    {HAL_CAN_ERROR_EWG, CAN_ERROR_BUS_WARNING},  {HAL_CAN_ERROR_ACK, CAN_ERROR_ACK},
    {HAL_CAN_ERROR_STF, CAN_ERROR_STUFF},        {HAL_CAN_ERROR_FOR, CAN_ERROR_FORM},
    {HAL_CAN_ERROR_BR, CAN_ERROR_BIT_RECESSIVE}, {HAL_CAN_ERROR_BD, CAN_ERROR_BIT_DOMINANT},
    {HAL_CAN_ERROR_CRC, CAN_ERROR_CRC},
};

#define CAN_ERROR_MAP_SIZE (sizeof(s_errorMap) / sizeof(s_errorMap[0]))

/**
 * @brief Classify the current HAL error flags for a handle
 * @param hcan Handle
 * @retval CAN_ErrorType Most severe live error, or CAN_ERROR_NONE
 */
CAN_ErrorType CAN_GetErrorType(CAN_Handle_t *hcan) {
    uint32_t error = 0;

    if (hcan == NULL) {
        return CAN_ERROR_NONE;
    }

    error = HAL_CAN_GetError(&hcan->hal);

    for (size_t i = 0; i < CAN_ERROR_MAP_SIZE; i++) {
        if (error & s_errorMap[i].hal_flag) {
            return s_errorMap[i].error;
        }
    }

    return CAN_ERROR_NONE;
}

/**
 * @brief Record an error against a handle
 * @param hcan Handle
 * @param error Error to record
 */
void CAN_RecordError(CAN_Handle_t *hcan, CAN_ErrorType error) {
    if (hcan == NULL) {
        return;
    }

    hcan->status.error_count++;
    hcan->status.last_error = error;
}

/**
 * @brief Bump transmit counter for a handle
 * @param hcan Handle
 */
void CAN_IncrementTxCount(CAN_Handle_t *hcan) {
    if (hcan != NULL) {
        hcan->status.tx_count++;
    }
}

/**
 * @brief Bump receive counter for a handle
 * @param hcan Handle
 */
void CAN_IncrementRxCount(CAN_Handle_t *hcan) {
    if (hcan != NULL) {
        hcan->status.rx_count++;
    }
}

/* HAL low-level init --------------------------------------------------------*/

/**
 * @brief HAL MSP init — called by HAL_CAN_Init() to enable clocks and configure pins
 * @param hcan_ptr HAL handle whose Instance selects CAN1 or CAN2
 * @note  Resolves the driver handle via the registry to find the pin config;
 *        CAN2 is a slave of CAN1 and requires both clocks.
 */
void HAL_CAN_MspInit(CAN_HandleTypeDef *hcan_ptr) {
    GPIO_InitTypeDef gpio = {0};
    CAN_Handle_t *hcan = CAN_FromInstance(hcan_ptr->Instance);
    const CAN_PinConfig *pins = NULL;

    if (hcan == NULL) {
        return;
    }

    if (hcan_ptr->Instance == CAN1) {
        __HAL_RCC_CAN1_CLK_ENABLE();
    }
    else {
        /* CAN2 is a slave of CAN1 and has no clock of its own to gate. */
        __HAL_RCC_CAN1_CLK_ENABLE();
        __HAL_RCC_CAN2_CLK_ENABLE();
    }

    pins = &hcan->config.pins;

    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = (pins->af != 0U) ? pins->af : CAN_DEFAULT_AF;

    gpio.Pin = pins->tx_pin;
    if (GPIO_Driver_Pin_Init(pins->tx_port, &gpio) != HAL_OK) {
        log_error("CAN: TX pin init failed");
    }

    gpio.Pin = pins->rx_pin;
    if (GPIO_Driver_Pin_Init(pins->rx_port, &gpio) != HAL_OK) {
        log_error("CAN: RX pin init failed");
    }
}

/**
 * @brief HAL MSP deinit — called by HAL_CAN_DeInit() to release clocks and pins
 * @param hcan_ptr HAL handle whose Instance selects CAN1 or CAN2
 */
void HAL_CAN_MspDeInit(CAN_HandleTypeDef *hcan_ptr) {
    CAN_Handle_t *hcan = CAN_FromInstance(hcan_ptr->Instance);

    if (hcan_ptr->Instance == CAN1) {
        __HAL_RCC_CAN1_CLK_DISABLE();
    }
    else if (hcan_ptr->Instance == CAN2) {
        __HAL_RCC_CAN2_CLK_DISABLE();
    }
    else {
        return;
    }

    if (hcan == NULL) {
        return;
    }

    (void)GPIO_Driver_Pin_DeInit(hcan->config.pins.tx_port, hcan->config.pins.tx_pin);
    (void)GPIO_Driver_Pin_DeInit(hcan->config.pins.rx_port, hcan->config.pins.rx_pin);
}

/**
 * @brief Initialize a CAN handle
 * @param hcan Handle to initialize (must be zeroed or uninitialized)
 * @param config Configuration to apply; instance defaults to CAN1 when NULL
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on failure
 */
HAL_StatusTypeDef CAN_Init(CAN_Handle_t *hcan, const CAN_Config *config) {
    HAL_StatusTypeDef status = HAL_OK;
    CAN_TypeDef *instance = NULL;
    int index = 0;

    if (hcan == NULL || config == NULL) {
        return HAL_ERROR;
    }

    if (config->pins.tx_port == NULL || config->pins.rx_port == NULL || config->pins.tx_pin == 0U ||
        config->pins.rx_pin == 0U) {
        log_error("CAN: no TX/RX pins given, and this board has no free pair to assume");
        return HAL_ERROR;
    }

    instance = (config->instance != NULL) ? config->instance : CAN1;
    index = CAN_InstanceIndex(instance);
    if (index < 0) {
        log_error("CAN: unknown CAN instance");
        return HAL_ERROR;
    }

    log_debug("CAN: Initializing CAN");

    memset(hcan, 0, sizeof(*hcan));
    hcan->config = *config;
    hcan->config.instance = instance;

    if (hcan->config.filter_bank_count == 0U) {
        hcan->config.filter_bank_count = CAN_DEFAULT_FILTER_BANKS;
    }
    if (hcan->config.slave_start_bank == 0U) {
        hcan->config.slave_start_bank = hcan->config.filter_bank_count;
    }

    hcan->hal.Instance = instance;
    hcan->hal.Init.Mode = config->mode;
    hcan->hal.Init.AutoRetransmission = config->auto_retransmission ? ENABLE : DISABLE;
    hcan->hal.Init.AutoBusOff = config->auto_bus_off_recovery ? ENABLE : DISABLE;
    hcan->hal.Init.TimeTriggeredMode = config->time_triggered_comm ? ENABLE : DISABLE;

    status = CAN_ApplyBaudRate(&hcan->hal, &hcan->config);
    if (status != HAL_OK) {
        log_error("CAN: no bit timing fits this APB1 clock at the requested baud rate");
        return status;
    }

    /* MspInit runs inside HAL_CAN_Init and looks the handle up by instance. */
    s_registry[index] = hcan;

    status = HAL_CAN_Init(&hcan->hal);
    if (status != HAL_OK) {
        s_registry[index] = NULL;
        return status;
    }

    status = HAL_CAN_Start(&hcan->hal);
    if (status != HAL_OK) {
        s_registry[index] = NULL;
        return status;
    }

    hcan->status.initialized = true;

    log_debug("CAN: CAN initialized successfully");

    return HAL_OK;
}

/**
 * @brief Release a CAN handle and unregister it
 * @param hcan Handle to release
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on failure
 */
HAL_StatusTypeDef CAN_DeInit(CAN_Handle_t *hcan) {
    HAL_StatusTypeDef status = HAL_OK;
    int index = 0;

    if (hcan == NULL || !hcan->status.initialized) {
        return HAL_ERROR;
    }

    status = HAL_CAN_Stop(&hcan->hal);
    if (status != HAL_OK) {
        return status;
    }

    /* MspDeInit still needs the registry entry to find the pins. */
    status = HAL_CAN_DeInit(&hcan->hal);
    if (status != HAL_OK) {
        return status;
    }

    index = CAN_InstanceIndex(hcan->hal.Instance);
    if (index >= 0) {
        s_registry[index] = NULL;
    }

    memset(&hcan->status, 0, sizeof(hcan->status));

    hcan->tx_callback = NULL;
    hcan->rx_callback = NULL;
    hcan->error_callback = NULL;

    return HAL_OK;
}

/**
 * @brief Switch operating mode without full reinit
 * @param hcan Handle (must be initialized)
 * @param mode New operating mode
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on failure
 */
HAL_StatusTypeDef CAN_SetMode(CAN_Handle_t *hcan, CAN_OperatingMode mode) {
    HAL_StatusTypeDef status = HAL_OK;
    uint32_t previous_mode = 0;

    if (hcan == NULL || !hcan->status.initialized) {
        return HAL_ERROR;
    }

    previous_mode = hcan->hal.Init.Mode;

    status = HAL_CAN_Stop(&hcan->hal);
    if (status != HAL_OK) {
        return status;
    }

    hcan->hal.Init.Mode = mode;

    status = HAL_CAN_Init(&hcan->hal);
    if (status != HAL_OK) {
        hcan->hal.Init.Mode = previous_mode;
        hcan->status.initialized = false;
        return status;
    }

    status = HAL_CAN_Start(&hcan->hal);
    if (status != HAL_OK) {
        hcan->status.initialized = false;
    }

    return status;
}

/**
 * @brief Snapshot live bus status
 * @param hcan Handle
 * @param status Output snapshot
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on failure
 */
HAL_StatusTypeDef CAN_GetStatus(CAN_Handle_t *hcan, CAN_Status *status) {
    CAN_ErrorType live = CAN_ERROR_NONE;

    if (hcan == NULL || status == NULL) {
        return HAL_ERROR;
    }

    memcpy(status, &hcan->status, sizeof(CAN_Status));

    status->tx_mailbox_free = HAL_CAN_GetTxMailboxesFreeLevel(&hcan->hal);
    status->rx_messages_pending = HAL_CAN_GetRxFifoFillLevel(&hcan->hal, CAN_RX_FIFO0) +
                                  HAL_CAN_GetRxFifoFillLevel(&hcan->hal, CAN_RX_FIFO1);

    /* Only override the recorded error while the condition is still live;
       otherwise the last fault would vanish as soon as HAL cleared it. */
    live = CAN_GetErrorType(hcan);
    if (live != CAN_ERROR_NONE) {
        status->last_error = live;
    }

    return HAL_OK;
}

/**
 * @brief Clear latched error flags and counters
 * @param hcan Handle
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on failure
 */
HAL_StatusTypeDef CAN_ClearErrors(CAN_Handle_t *hcan) {
    if (hcan == NULL) {
        return HAL_ERROR;
    }

    /* Only the FIFO overrun flags are writable. EWG/EPV/BOF are read-only
       status bits in ESR that the controller owns; attempting to clear them
       expands to a no-op expression. */
    __HAL_CAN_CLEAR_FLAG(&hcan->hal, CAN_FLAG_FOV0);
    __HAL_CAN_CLEAR_FLAG(&hcan->hal, CAN_FLAG_FOV1);

    hcan->hal.ErrorCode = HAL_CAN_ERROR_NONE;

    hcan->status.error_count = 0;
    hcan->status.last_error = CAN_ERROR_NONE;

    return HAL_OK;
}
