/**
 * @file ts_touch.c
 * @brief Touch acquisition: reading, mapping and smoothing a touch point
 */

#include "ts_touch.h"
#include "ts_io.h"
#include "ts_calibration.h"
#include "ts_stmpe811.h"
#include "log.h"

#define TS_FILTER_MOVE_THRESHOLD    5       /*!< Manhattan distance below which a move is treated as jitter */
#define TS_PRESSURE_MAX_VALUE       0xFFU
#define TS_PRESSURE_SCALE           255U

/* Private types -------------------------------------------------------------*/

/** Outcome of one acquisition attempt. */
typedef enum {
    TS_ACQUIRE_TOUCHED = 0,     /*!< A point was read and mapped */
    TS_ACQUIRE_IDLE,            /*!< Nothing on the panel */
    TS_ACQUIRE_FAILED           /*!< Controller did not answer */
} TS_AcquireResult_t;

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Read one raw sample from the touch FIFO
 * @param hts Touchscreen handle
 * @param rawX Destination raw X
 * @param rawY Destination raw Y
 * @param pressure Optional destination for the raw Z byte
 * @return TS_StatusTypeDef Status of the operation
 */
static TS_StatusTypeDef TS_ReadRawCoordinates(TS_HandleTypeDef *hts,
                                              uint16_t *rawX, uint16_t *rawY, uint16_t *pressure)
{
    uint8_t data[4] = {0};

    if (TS_ReadRegisterMulti(hts, STMPE811_REG_TSC_DATA_NON_INC, data, sizeof(data)) != TS_OK) {
        return TS_COMMUNICATION_ERROR;
    }

    uint32_t xyz = ((uint32_t)data[0] << 24) |
                   ((uint32_t)data[1] << 16) |
                   ((uint32_t)data[2] << 8) |
                   (uint32_t)data[3];

    /* 12-bit X and Y packed above the 8-bit Z, per the STMPE811 datasheet. */
    *rawX = (uint16_t)((xyz >> 20) & 0x0FFFU);
    *rawY = (uint16_t)((xyz >> 8) & 0x0FFFU);
    if (pressure != NULL) {
        *pressure = (uint16_t)(xyz & 0xFFU);
    }

    log_debug("Raw X: %u, Raw Y: %u, Pressure: %u", *rawX, *rawY, pressure ? *pressure : 0);

    TS_ResetFifo(hts);
    return TS_OK;
}

/**
 * @brief Hold the previous point until the finger moves far enough to be real movement
 * @param hts Touchscreen handle carrying the filter state
 * @param x In/out display X
 * @param y In/out display Y
 */
static void TS_FilterCoordinates(TS_HandleTypeDef *hts, uint16_t *x, uint16_t *y)
{
    int32_t newX = (int32_t)*x;
    int32_t newY = (int32_t)*y;

    int32_t xDiff = newX - (int32_t)hts->FilterX;
    int32_t yDiff = newY - (int32_t)hts->FilterY;
    if (xDiff < 0) { xDiff = -xDiff; }
    if (yDiff < 0) { yDiff = -yDiff; }

    if ((xDiff + yDiff) > TS_FILTER_MOVE_THRESHOLD) {
        hts->FilterX = (uint16_t)newX;
        hts->FilterY = (uint16_t)newY;
    }

    *x = hts->FilterX;
    *y = hts->FilterY;

    log_debug("Filter X = %u, Filter Y = %u", *x, *y);
}

/**
 * @brief Read, map and smooth one touch point
 * @param hts Touchscreen handle
 * @param x Destination display X
 * @param y Destination display Y
 * @return TS_AcquireResult_t Whether a point was produced
 */
static TS_AcquireResult_t TS_AcquirePoint(TS_HandleTypeDef *hts, uint16_t *x, uint16_t *y)
{
    uint8_t status = 0;
    if (TS_ReadRegister(hts, STMPE811_REG_TSC_CTRL, &status) != TS_OK) {
        return TS_ACQUIRE_FAILED;
    }

    if ((status & (uint8_t)STMPE811_TS_CTRL_STATUS) == 0) {
        TS_ResetFifo(hts);
        return TS_ACQUIRE_IDLE;
    }

    uint16_t rawX = 0;
    uint16_t rawY = 0;
    if (TS_ReadRawCoordinates(hts, &rawX, &rawY, NULL) != TS_OK) {
        return TS_ACQUIRE_FAILED;
    }

    if (TS_MapToDisplay(hts, rawX, rawY, x, y) != TS_OK) {
        return TS_ACQUIRE_FAILED;
    }

    TS_FilterCoordinates(hts, x, y);
    return TS_ACQUIRE_TOUCHED;
}

/* Public functions ----------------------------------------------------------*/

TS_StatusTypeDef TS_GetTouchData(TS_HandleTypeDef *hts, TS_TouchDataTypeDef *touch_data)
{
    if (hts == NULL || touch_data == NULL) {
        return TS_INVALID_PARAM;
    }

    *touch_data = hts->TouchData;
    return TS_OK;
}

TS_StatusTypeDef TS_GetSingleTouch(TS_HandleTypeDef *hts, uint16_t *xPos, uint16_t *yPos)
{
    if (hts == NULL || xPos == NULL || yPos == NULL) {
        return TS_INVALID_PARAM;
    }

    *xPos = 0;
    *yPos = 0;

    TS_StatusTypeDef ready = TS_CheckReady(hts);
    if (ready != TS_OK) {
        return ready;
    }

    uint16_t x = 0;
    uint16_t y = 0;
    if (TS_AcquirePoint(hts, &x, &y) != TS_ACQUIRE_TOUCHED) {
        return TS_ERROR;
    }

    *xPos = x;
    *yPos = y;
    return TS_OK;
}

TS_StatusTypeDef TS_GetTouchState(TS_HandleTypeDef *hts, uint16_t *x, uint16_t *y, uint8_t *pressed)
{
    if (hts == NULL || x == NULL || y == NULL || pressed == NULL) {
        return TS_INVALID_PARAM;
    }

    *x = 0;
    *y = 0;
    *pressed = 0;

    TS_StatusTypeDef ready = TS_CheckReady(hts);
    if (ready != TS_OK) {
        return ready;
    }

    uint16_t touchX = 0;
    uint16_t touchY = 0;
    switch (TS_AcquirePoint(hts, &touchX, &touchY)) {
        case TS_ACQUIRE_TOUCHED:
            *x = touchX;
            *y = touchY;
            *pressed = 1;
            return TS_OK;

        case TS_ACQUIRE_IDLE:
            return TS_OK;   /* An untouched panel is a normal result here */

        default:
            return TS_ERROR;
    }
}

bool TS_IsTouched(TS_HandleTypeDef *hts)
{
    if (TS_CheckReady(hts) != TS_OK) {
        return false;
    }

    uint8_t fifoSize = 0;
    if (TS_ReadRegister(hts, STMPE811_REG_FIFO_SIZE, &fifoSize) != TS_OK) {
        return false;
    }

    return (fifoSize > 0);
}

uint8_t TS_GetTouchCount(TS_HandleTypeDef *hts)
{
    if (hts == NULL) {
        return 0;
    }

    return hts->TouchData.TouchCount;
}

TS_StatusTypeDef TS_GetPressure(TS_HandleTypeDef *hts, uint16_t *pressure)
{
    if (hts == NULL || pressure == NULL) {
        return TS_INVALID_PARAM;
    }

    uint8_t data = 0;
    if (TS_ReadRegister(hts, STMPE811_REG_TSC_DATA_Z, &data) != TS_OK) {
        return TS_COMMUNICATION_ERROR;
    }

    *pressure = (uint16_t)data;
    if (*pressure > 0) {
        *pressure = (uint16_t)((*pressure * TS_PRESSURE_SCALE) / TS_PRESSURE_MAX_VALUE);
    }

    return TS_OK;
}
