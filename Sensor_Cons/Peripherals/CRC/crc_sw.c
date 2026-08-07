/**
  ******************************************************************************
  * @file    crc_sw.c
  * @brief   Software CRC implementation
  ******************************************************************************
  */

#include "crc_sw.h"

#define CRC_MSB_MASK                 0x80000000U
#define CRC_SHIFT_24                 24U
#define CRC_BITS_PER_WORD            32U
#define CRC_BITS_PER_BYTE            8U

static uint32_t CRC_Reflect(uint32_t value, uint8_t bits)
{
    uint32_t reflected = 0U;

    for (uint8_t i = 0U; i < bits; i++) {
        if ((value & (1UL << i)) != 0U) {
            reflected |= (1UL << (bits - 1U - i));
        }
    }

    return reflected;
}

static uint32_t CRC_ShiftBits(uint32_t crc, uint32_t polynomial, uint8_t bits)
{
    for (uint8_t i = 0U; i < bits; i++) {
        crc = ((crc & CRC_MSB_MASK) != 0U) ? ((crc << 1) ^ polynomial) : (crc << 1);
    }

    return crc;
}

uint32_t CRC_SoftwareUpdate(const CRC_Config *config, uint32_t seed,
                            const uint8_t *data, uint32_t size)
{
    uint32_t crc = seed;

    if (config == NULL || data == NULL) {
        return seed;
    }

    for (uint32_t i = 0U; i < size; i++) {
        uint32_t byte = data[i];

        if (config->input_reverse) {
            byte = CRC_Reflect(byte, CRC_BITS_PER_BYTE);
        }

        crc ^= byte << CRC_SHIFT_24;
        crc = CRC_ShiftBits(crc, config->polynomial, CRC_BITS_PER_BYTE);
    }

    return crc;
}

uint32_t CRC_SoftwareUpdate32(const CRC_Config *config, uint32_t seed,
                              const uint32_t *data, uint32_t size)
{
    uint32_t crc = seed;

    if (config == NULL || data == NULL) {
        return seed;
    }

    for (uint32_t i = 0U; i < size; i++) {
        uint32_t word = data[i];

        if (config->input_reverse) {
            word = CRC_Reflect(word, CRC_BITS_PER_WORD);
        }

        crc ^= word;
        crc = CRC_ShiftBits(crc, config->polynomial, CRC_BITS_PER_WORD);
    }

    return crc;
}

uint32_t CRC_SoftwareFinalize(const CRC_Config *config, uint32_t crc)
{
    if (config == NULL || !config->output_reverse) {
        return crc;
    }

    return CRC_Reflect(crc, CRC_BITS_PER_WORD);
}
