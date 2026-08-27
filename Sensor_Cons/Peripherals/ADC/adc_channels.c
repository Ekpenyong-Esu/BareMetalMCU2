/**
 * @file    adc_channels.c
 * @brief   ADC channel to GPIO mapping implementation
 */

/* Includes ------------------------------------------------------------------*/
#include "adc_channels.h"

#include "gpio.h"

#include <stddef.h>

/* Private types -------------------------------------------------------------*/

typedef struct {
    uint32_t channel;       /*!< HAL channel constant */
    GPIO_TypeDef* port;     /*!< Backing port, NULL for internal channels */
    uint16_t pin;           /*!< Backing pin */
    const char* name;       /*!< Display name */
} ADC_ChannelMapEntry_t;

/* Private constants ---------------------------------------------------------*/

/* Board-fixed mapping: each ADC channel is hard-wired to one pin. Internal
 * channels (temp sensor, VREF, VBAT) have no pin, hence NULL port. */
static const ADC_ChannelMapEntry_t adc_channel_map[] = {
    { ADC_CHANNEL_0,  GPIOA, GPIO_PIN_0, "PA0" },
    { ADC_CHANNEL_1,  GPIOA, GPIO_PIN_1, "PA1" },
    { ADC_CHANNEL_2,  GPIOA, GPIO_PIN_2, "PA2" },
    { ADC_CHANNEL_3,  GPIOA, GPIO_PIN_3, "PA3" },
    { ADC_CHANNEL_4,  GPIOA, GPIO_PIN_4, "PA4" },
    { ADC_CHANNEL_5,  GPIOA, GPIO_PIN_5, "PA5" },
    { ADC_CHANNEL_6,  GPIOA, GPIO_PIN_6, "PA6" },
    { ADC_CHANNEL_7,  GPIOA, GPIO_PIN_7, "PA7" },
    { ADC_CHANNEL_8,  GPIOB, GPIO_PIN_0, "PB0" },
    { ADC_CHANNEL_9,  GPIOB, GPIO_PIN_1, "PB1" },
    { ADC_CHANNEL_10, GPIOC, GPIO_PIN_0, "PC0" },
    { ADC_CHANNEL_11, GPIOC, GPIO_PIN_1, "PC1" },
    { ADC_CHANNEL_12, GPIOC, GPIO_PIN_2, "PC2" },
    { ADC_CHANNEL_13, GPIOC, GPIO_PIN_3, "PC3" },
    { ADC_CHANNEL_14, GPIOC, GPIO_PIN_4, "PC4" },
    { ADC_CHANNEL_15, GPIOC, GPIO_PIN_5, "PC5" },
    { ADC_CHANNEL_TEMPSENSOR, NULL, 0, "TEMP" },
    { ADC_CHANNEL_VREFINT,    NULL, 0, "VREF" },
    { ADC_CHANNEL_VBAT,       NULL, 0, "VBAT" },
};

#define ADC_CHANNEL_MAP_COUNT \
    (sizeof(adc_channel_map) / sizeof(adc_channel_map[0]))

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Find the map entry for a channel
 * @param   channel HAL channel constant
 * @retval  const ADC_ChannelMapEntry_t* Entry, or NULL when unknown
 * @note    Linear lookup; the table is small so no hashing is needed.
 */
static const ADC_ChannelMapEntry_t* ADC_FindChannel(uint32_t channel)
{
    for (uint32_t i = 0; i < ADC_CHANNEL_MAP_COUNT; i++) {
        if (adc_channel_map[i].channel == channel) {
            return &adc_channel_map[i];
        }
    }
    return NULL;
}

/* Exported functions --------------------------------------------------------*/

HAL_StatusTypeDef ADC_ConfigureChannelGpio(uint32_t channel)
{
    const ADC_ChannelMapEntry_t* entry = ADC_FindChannel(channel);
    if (entry == NULL) {
        return HAL_ERROR;
    }

    if (entry->port == NULL) {
        return HAL_OK;  /* Internal channel, no pin to configure */
    }

    /* The GPIO driver enables the port clock. Analog mode disconnects the
     * digital input buffer, which is what the ADC needs on the pin. */
    GPIO_InitTypeDef init = {0};
    init.Pin = entry->pin;
    init.Mode = GPIO_MODE_ANALOG;
    init.Pull = GPIO_NOPULL;
    GPIO_Driver_Pin_Init(entry->port, &init);

    return HAL_OK;
}

/* For log/debug output; not used for control flow. */
const char* ADC_GetChannelName(uint32_t channel)
{
    const ADC_ChannelMapEntry_t* entry = ADC_FindChannel(channel);
    return (entry != NULL) ? entry->name : "UNKNOWN";
}
