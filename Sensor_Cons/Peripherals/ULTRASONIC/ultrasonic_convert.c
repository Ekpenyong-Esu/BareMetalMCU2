/**
  ******************************************************************************
  * @file    ultrasonic_convert.c
  * @brief   Echo width to distance conversion
  ******************************************************************************
  */

#include "ultrasonic_convert.h"

/* v = 331.3 + 0.6 * T  (m/s), expressed in mm/s to stay in integers. */
#define ULTRASONIC_SPEED_BASE_MM_S      331300
#define ULTRASONIC_SPEED_PER_DEGREE     600

#define ULTRASONIC_US_PER_SECOND        1000000U

uint32_t ULTRASONIC_SpeedOfSound(int8_t temperature)
{
    int32_t speed = ULTRASONIC_SPEED_BASE_MM_S +
                    (ULTRASONIC_SPEED_PER_DEGREE * (int32_t)temperature);

    return (speed > 0) ? (uint32_t)speed : 0U;
}

uint16_t ULTRASONIC_EchoToDistance(uint32_t echoUs, int8_t temperature)
{
    uint64_t distance;

    /* Round trip, hence the factor of two. */
    distance = ((uint64_t)echoUs * ULTRASONIC_SpeedOfSound(temperature)) /
               (2ULL * ULTRASONIC_US_PER_SECOND);

    if (distance > ULTRASONIC_ABS_MAX_DISTANCE) {
        return ULTRASONIC_ABS_MAX_DISTANCE;
    }

    return (uint16_t)distance;
}

uint32_t ULTRASONIC_EchoWidth(uint32_t start, uint32_t end, uint32_t period)
{
    if (end >= start) {
        return end - start;
    }

    /* The counter wrapped: it reloads to 0 on the tick after `period`. */
    return (period - start) + end + 1U;
}

bool ULTRASONIC_IsValidDistance(const ULTRASONIC_Handle_t *hultra, uint16_t distance)
{
    if (hultra == NULL) {
        return false;
    }

    return (distance >= hultra->config.minDistance && distance <= hultra->config.maxDistance);
}
