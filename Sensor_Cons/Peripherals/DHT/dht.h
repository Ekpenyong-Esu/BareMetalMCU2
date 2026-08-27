#ifndef DHT_H
#define DHT_H

#include "stm32f4xx.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DHT_TYPE_DHT11 = 0,
    DHT_TYPE_DHT22
} DHT_Type_t;

typedef struct {
    DHT_Type_t type;
    GPIO_TypeDef *port;
    uint16_t pin;
    uint32_t lastReadMs;
    bool valid;
    float humidity;
    float temperatureC;
} DHT_Handle_t;

/* Records the pin/type this sensor is wired to; performs no bus activity. */
HAL_StatusTypeDef DHT_Init(DHT_Handle_t *hdht, DHT_Type_t type, GPIO_TypeDef *port, uint16_t pin);
/* Runs the bit-banged read sequence and latches the result into hdht. */
HAL_StatusTypeDef DHT_Read(DHT_Handle_t *hdht);
/* Whether the last DHT_Read() succeeded. */
bool DHT_IsValid(const DHT_Handle_t *hdht);
float DHT_GetHumidity(const DHT_Handle_t *hdht);
float DHT_GetTemperatureC(const DHT_Handle_t *hdht);

#ifdef __cplusplus
}
#endif

#endif /* DHT_H */
