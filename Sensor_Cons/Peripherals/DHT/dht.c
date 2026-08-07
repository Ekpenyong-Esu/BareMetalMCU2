#include "dht.h"
#include "gpio.h"

// A data bit longer than this is a '1' (0-bits are ~27us, 1-bits ~70us)
#define DHT_BIT_ONE_THRESHOLD_US  40U

// DWT-based microsecond delay helpers
static inline uint32_t dht_us_to_ticks(uint32_t us)
{
    return (SystemCoreClock / 1000000U) * us;
}

static inline void dht_delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = dht_us_to_ticks(us);
    while ((DWT->CYCCNT - start) < ticks) {
        __NOP();
    }
}

static inline void dht_line_output(GPIO_TypeDef *port, uint16_t pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_Driver_Pin_Init(port, &GPIO_InitStruct);
}

static inline void dht_line_input(GPIO_TypeDef *port, uint16_t pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    /* Keeps the line defined if the module has no external pull-up. */
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_Driver_Pin_Init(port, &GPIO_InitStruct);
}

static inline bool dht_wait_level(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState level, uint32_t timeout_us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = dht_us_to_ticks(timeout_us);
    while ((DWT->CYCCNT - start) < ticks) {
        if (GPIO_Driver_ReadPin(port, pin) == level) {
            return true;
        }
    }
    return false;
}

HAL_StatusTypeDef DHT_Init(DHT_Handle_t *hdht, DHT_Type_t type, GPIO_TypeDef *port, uint16_t pin)
{
    if (hdht == NULL || port == NULL) {
        return HAL_ERROR;
    }

    // Enable DWT cycle counter for microsecond timing
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    }
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    // Every delay here spins on CYCCNT, so a stalled counter would hang forever
    if (DWT->CYCCNT == 0U) {
        return HAL_ERROR;
    }

    hdht->type = type;
    hdht->port = port;
    hdht->pin = pin;
    hdht->lastReadMs = 0;
    hdht->valid = false;
    hdht->humidity = 0.0f;
    hdht->temperatureC = 0.0f;

    dht_line_output(port, pin);
    GPIO_Driver_WritePin(port, pin, GPIO_PIN_SET);

    return HAL_OK;
}

// Leaves the bus idle high so the sensor can see the next start pulse
static HAL_StatusTypeDef dht_finish(DHT_Handle_t *hdht, HAL_StatusTypeDef status)
{
    dht_line_output(hdht->port, hdht->pin);
    GPIO_Driver_WritePin(hdht->port, hdht->pin, GPIO_PIN_SET);
    return status;
}

HAL_StatusTypeDef DHT_Read(DHT_Handle_t *hdht)
{
    if (hdht == NULL) {
        return HAL_ERROR;
    }

    // Enforce minimum interval (DHT11: 1s, DHT22: 2s recommended)
    uint32_t now = HAL_GetTick();
    uint32_t minInterval = (hdht->type == DHT_TYPE_DHT11) ? 1000U : 2000U;
    if ((now - hdht->lastReadMs) < minInterval) {
        return HAL_BUSY;
    }

    // Any early return below leaves no fresh sample
    hdht->valid = false;
    // Rate-limit failed attempts too, so a missing sensor is not hammered
    hdht->lastReadMs = now;

    uint8_t data[5] = {0};

    // Start signal: pull low
    dht_line_output(hdht->port, hdht->pin);
    GPIO_Driver_WritePin(hdht->port, hdht->pin, GPIO_PIN_RESET);
    if (hdht->type == DHT_TYPE_DHT11) {
        HAL_Delay(18); // 18 ms
    } else {
        HAL_Delay(2);  // >1 ms for DHT22
    }
    GPIO_Driver_WritePin(hdht->port, hdht->pin, GPIO_PIN_SET);
    dht_delay_us(30);

    // Switch to input
    dht_line_input(hdht->port, hdht->pin);

    /* The response and the 40 data bits are decoded from pulse widths of a few
       tens of microseconds, so any interrupt taken here corrupts the frame.
       DWT keeps counting with interrupts masked, and the section is ~5 ms. */
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    HAL_StatusTypeDef status = HAL_OK;

    // Wait for sensor response: 80us low, 80us high
    if (!dht_wait_level(hdht->port, hdht->pin, GPIO_PIN_RESET, 100) ||
        !dht_wait_level(hdht->port, hdht->pin, GPIO_PIN_SET, 100) ||
        !dht_wait_level(hdht->port, hdht->pin, GPIO_PIN_RESET, 100)) {
        status = HAL_TIMEOUT;
    }

    // Read 40 bits
    for (uint8_t i = 0; (status == HAL_OK) && (i < 40); i++) {
        // Wait for line to go high (start of bit)
        if (!dht_wait_level(hdht->port, hdht->pin, GPIO_PIN_SET, 70)) {
            status = HAL_TIMEOUT;
            break;
        }
        uint32_t start = DWT->CYCCNT;
        // Wait for line to go low (end of bit)
        if (!dht_wait_level(hdht->port, hdht->pin, GPIO_PIN_RESET, 100)) {
            status = HAL_TIMEOUT;
            break;
        }
        uint32_t widthTicks = DWT->CYCCNT - start;
        uint32_t widthUs = widthTicks / (SystemCoreClock / 1000000U);
        if (widthUs > DHT_BIT_ONE_THRESHOLD_US) {
            data[i / 8] |= (1U << (7 - (i % 8)));
        }
    }

    __set_PRIMASK(primask);

    if (status != HAL_OK) {
        return dht_finish(hdht, status);
    }

    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) {
        return dht_finish(hdht, HAL_ERROR);
    }

    if (hdht->type == DHT_TYPE_DHT11) {
        hdht->humidity = (float)data[0];
        hdht->temperatureC = (float)data[2];
    } else {
        uint16_t rawHum = (uint16_t)((data[0] << 8) | data[1]);
        uint16_t rawTemp = (uint16_t)((data[2] << 8) | data[3]);

        hdht->humidity = rawHum / 10.0f;

        // DHT22 temperature is sign-magnitude, not two's complement
        float tempC = (float)(rawTemp & 0x7FFFU) / 10.0f;
        hdht->temperatureC = (rawTemp & 0x8000U) ? -tempC : tempC;
    }

    hdht->valid = true;
    return dht_finish(hdht, HAL_OK);
}

bool DHT_IsValid(const DHT_Handle_t *hdht)
{
    return (hdht != NULL) && hdht->valid;
}

float DHT_GetHumidity(const DHT_Handle_t *hdht)
{
    return (hdht != NULL) ? hdht->humidity : 0.0f;
}

float DHT_GetTemperatureC(const DHT_Handle_t *hdht)
{
    return (hdht != NULL) ? hdht->temperatureC : 0.0f;
}
