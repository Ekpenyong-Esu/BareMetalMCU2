/**
 * @file rng_generate.c
 * @brief Raw random word and byte generation
 */

#include "rng_generate.h"
#include "rng_core.h"

#define RNG_BYTES_PER_WORD  4U

RNG_StatusTypeDef RNG_Generate(uint32_t *randomNumber)
{
    RNG_HandleTypeDef *hrng = RNG_GetHandle();
    RNG_StatusTypeDef status;
    HAL_StatusTypeDef halStatus;

    if (randomNumber == NULL) {
        return RNG_ERROR;
    }

    if (hrng == NULL) {
        return RNG_NOT_READY;
    }

    status = RNG_GetErrorStatus();
    if (status != RNG_OK) {
        return status;
    }

    halStatus = HAL_RNG_GenerateRandomNumber(hrng, randomNumber);
    if (halStatus != HAL_OK) {
        return (halStatus == HAL_TIMEOUT) ? RNG_TIMEOUT : RNG_ERROR;
    }

    /* A seed error latched during the draw makes the word just read invalid. */
    status = RNG_GetErrorStatus();
    if (status == RNG_SEED_ERROR) {
        return status;
    }

    return RNG_OK;
}

RNG_StatusTypeDef RNG_GenerateBuffer(uint32_t *buffer, uint32_t count)
{
    if (buffer == NULL || count == 0U) {
        return RNG_ERROR;
    }

    for (uint32_t i = 0; i < count; i++) {
        RNG_StatusTypeDef status = RNG_Generate(&buffer[i]);
        if (status != RNG_OK) {
            return status;
        }
    }

    return RNG_OK;
}

RNG_StatusTypeDef RNG_GenerateBytes(uint8_t *buffer, uint32_t length)
{
    uint32_t bytesRemaining = length;
    uint32_t bufferIndex = 0;

    if (buffer == NULL || length == 0U) {
        return RNG_ERROR;
    }

    while (bytesRemaining > 0U) {
        uint32_t randomWord = 0;
        uint32_t bytesToCopy;

        RNG_StatusTypeDef status = RNG_Generate(&randomWord);
        if (status != RNG_OK) {
            return status;
        }

        bytesToCopy = (bytesRemaining >= RNG_BYTES_PER_WORD) ? RNG_BYTES_PER_WORD
                                                            : bytesRemaining;
        for (uint32_t i = 0; i < bytesToCopy; i++) {
            buffer[bufferIndex++] = (uint8_t)(randomWord >> (i * 8U));
        }

        bytesRemaining -= bytesToCopy;
    }

    return RNG_OK;
}
