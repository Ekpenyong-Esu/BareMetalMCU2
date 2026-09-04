/**
 * @file uart_ring_buffer.c
 * @brief Fixed-capacity byte queue
 */

#include "uart_ring_buffer.h"
#include <string.h>

void RingBuffer_Init(RingBuffer_t *ringBuffer) {
    ringBuffer->head = 0;
    ringBuffer->tail = 0;
    ringBuffer->count = 0;
    memset(ringBuffer->buffer, 0, sizeof(ringBuffer->buffer));
}

bool RingBuffer_Put(RingBuffer_t *ringBuffer, uint8_t data) {
    if (ringBuffer == NULL || RingBuffer_IsFull(ringBuffer)) {
        return false;
    }

    ringBuffer->buffer[ringBuffer->head] = data;
    ringBuffer->head = (ringBuffer->head + 1) % RING_BUFFER_SIZE;
    ringBuffer->count++;
    return true;
}

bool RingBuffer_Get(RingBuffer_t *ringBuffer, uint8_t *data) {
    if (ringBuffer == NULL || data == NULL || RingBuffer_IsEmpty(ringBuffer)) {
        return false;
    }

    *data = ringBuffer->buffer[ringBuffer->tail];
    ringBuffer->tail = (ringBuffer->tail + 1) % RING_BUFFER_SIZE;
    ringBuffer->count--;
    return true;
}

uint32_t RingBuffer_PutBytes(RingBuffer_t *ringBuffer, const uint8_t *data, uint32_t count) {
    if (ringBuffer == NULL || data == NULL) {
        return 0;
    }

    uint32_t stored = 0;
    while (stored < count && RingBuffer_Put(ringBuffer, data[stored])) {
        stored++;
    }

    return stored;
}

bool RingBuffer_GetBytes(RingBuffer_t *ringBuffer, uint8_t *data, uint32_t count) {
    if (ringBuffer == NULL || data == NULL || count == 0) {
        return false;
    }

    if (RingBuffer_Available(ringBuffer) < count) {
        return false;
    }

    for (uint32_t i = 0; i < count; i++) {
        RingBuffer_Get(ringBuffer, &data[i]);
    }

    return true;
}

uint32_t RingBuffer_Available(const RingBuffer_t *ringBuffer) {
    return ringBuffer->count;
}

bool RingBuffer_IsFull(const RingBuffer_t *ringBuffer) {
    return ringBuffer->count >= RING_BUFFER_SIZE;
}

bool RingBuffer_IsEmpty(const RingBuffer_t *ringBuffer) {
    return ringBuffer->count == 0;
}
