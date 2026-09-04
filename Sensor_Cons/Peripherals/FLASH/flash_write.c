/**
 ******************************************************************************
 * @file    flash_write.c
 * @brief   Flash programming operations
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "flash_write.h"
#include "flash_core.h"
#include "flash_read.h"
#include "flash_sector.h"

/* Private defines -----------------------------------------------------------*/

/** @brief Address alignment masks per program width */
#define FLASH_ALIGN_BYTE 0x00U
#define FLASH_ALIGN_HALFWORD 0x01U
#define FLASH_ALIGN_WORD 0x03U
#define FLASH_ALIGN_DOUBLEWORD 0x07U

/** @brief Largest word count that can be expressed in bytes without wrapping */
#define FLASH_MAX_WORD_COUNT (0xFFFFFFFFU / 4U)

/* Private function prototypes -----------------------------------------------*/
static FLASH_StatusTypeDef FLASH_ProgramScalar(uint32_t typeProgram, uint32_t address,
                                               uint64_t data, uint32_t alignMask);
static FLASH_StatusTypeDef FLASH_ProgramArray(uint32_t typeProgram, uint32_t address,
                                              const void *data, uint32_t count,
                                              uint32_t elementSize);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Unlock, program one scalar value, then re-lock
 * @details Shared body of the byte/half-word/word/double-word writers
 * @param   typeProgram HAL program type (FLASH_TYPEPROGRAM_xxx)
 * @param   address Flash address to write
 * @param   data Value to program
 * @param   alignMask Address bits that must be zero for this width
 * @retval  FLASH_StatusTypeDef Operation status
 */
static FLASH_StatusTypeDef FLASH_ProgramScalar(uint32_t typeProgram, uint32_t address,
                                               uint64_t data, uint32_t alignMask) {
    FLASH_StatusTypeDef status = FLASH_STATUS_OK;
    HAL_StatusTypeDef halStatus = HAL_OK;

    if (!FLASH_IsValidRange(address, alignMask + 1U) || (address & alignMask) != 0U) {
        return FLASH_STATUS_INVALID_ADDRESS;
    }

    status = FLASH_Unlock();
    if (status != FLASH_STATUS_OK) {
        return status;
    }

    FLASH_ClearErrorFlags();

    halStatus = HAL_FLASH_Program(typeProgram, address, data);

    /* Leaving flash unlocked is a hazard of its own, so a failed re-lock is
       reported unless the program already failed. */
    if (FLASH_Lock() != FLASH_STATUS_OK && halStatus == HAL_OK) {
        return FLASH_STATUS_ERROR;
    }

    return FLASH_ConvertHALStatus(halStatus);
}

/**
 * @brief   Program an array of equally sized elements and verify the result
 * @details Shared body of FLASH_WriteBuffer and FLASH_WriteBuffer32
 * @param   typeProgram HAL program type (FLASH_TYPEPROGRAM_xxx)
 * @param   address Start address in Flash
 * @param   data Pointer to the source elements
 * @param   count Number of elements
 * @param   elementSize Element size in bytes, 1 or 4
 * @retval  FLASH_StatusTypeDef Operation status
 */
static FLASH_StatusTypeDef FLASH_ProgramArray(uint32_t typeProgram, uint32_t address,
                                              const void *data, uint32_t count,
                                              uint32_t elementSize) {
    const uint8_t *bytes = (const uint8_t *)data;
    FLASH_StatusTypeDef status = FLASH_STATUS_OK;

    status = FLASH_Unlock();
    if (status != FLASH_STATUS_OK) {
        return status;
    }

    FLASH_ClearErrorFlags();

    for (uint32_t i = 0; i < count; i++) {
        const uint32_t offset = i * elementSize;
        uint64_t value = 0;

        for (uint32_t byteIdx = 0; byteIdx < elementSize; byteIdx++) {
            value |= (uint64_t)bytes[offset + byteIdx] << (8U * byteIdx);
        }

        if (HAL_FLASH_Program(typeProgram, address + offset, value) != HAL_OK) {
            (void)FLASH_Lock();
            return FLASH_STATUS_ERROR_PROGRAM;
        }
    }

    if (FLASH_Lock() != FLASH_STATUS_OK) {
        return FLASH_STATUS_ERROR;
    }

    /* Read back: a program that reports success can still leave a bit unset
       when the target was not erased first. */
    for (uint32_t i = 0; i < (count * elementSize); i++) {
        if (FLASH_ReadByte(address + i) != bytes[i]) {
            return FLASH_STATUS_ERROR_PROGRAM;
        }
    }

    return FLASH_STATUS_OK;
}

/* Public functions ----------------------------------------------------------*/

FLASH_StatusTypeDef FLASH_WriteByte(uint32_t address, uint8_t data) {
    return FLASH_ProgramScalar(FLASH_TYPEPROGRAM_BYTE, address, data, FLASH_ALIGN_BYTE);
}

FLASH_StatusTypeDef FLASH_WriteHalfWord(uint32_t address, uint16_t data) {
    return FLASH_ProgramScalar(FLASH_TYPEPROGRAM_HALFWORD, address, data, FLASH_ALIGN_HALFWORD);
}

FLASH_StatusTypeDef FLASH_WriteWord(uint32_t address, uint32_t data) {
    return FLASH_ProgramScalar(FLASH_TYPEPROGRAM_WORD, address, data, FLASH_ALIGN_WORD);
}

FLASH_StatusTypeDef FLASH_WriteDoubleWord(uint32_t address, uint64_t data) {
    return FLASH_ProgramScalar(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data, FLASH_ALIGN_DOUBLEWORD);
}

FLASH_StatusTypeDef FLASH_WriteBuffer(uint32_t address, const uint8_t *data, uint32_t length) {
    if (data == NULL || length == 0U) {
        return FLASH_STATUS_INVALID_PARAM;
    }

    if (!FLASH_IsValidRange(address, length)) {
        return FLASH_STATUS_INVALID_ADDRESS;
    }

    return FLASH_ProgramArray(FLASH_TYPEPROGRAM_BYTE, address, data, length, 1U);
}

FLASH_StatusTypeDef FLASH_WriteBuffer32(uint32_t address, const uint32_t *data, uint32_t count) {
    if (data == NULL || count == 0U || count > FLASH_MAX_WORD_COUNT) {
        return FLASH_STATUS_INVALID_PARAM;
    }

    if ((address & FLASH_ALIGN_WORD) != 0U || !FLASH_IsValidRange(address, count * 4U)) {
        return FLASH_STATUS_INVALID_ADDRESS;
    }

    return FLASH_ProgramArray(FLASH_TYPEPROGRAM_WORD, address, data, count, 4U);
}
