/**
 * @file uart_ring_buffer.h
 * @brief Fixed-capacity byte queue
 *
 * A plain container: it stores bytes and knows nothing about UART, so it has
 * no opinion on where the bytes came from and does no logging. The driver
 * embeds one per link so reception can outrun the reader.
 */

#ifndef UART_RING_BUFFER_H
#define UART_RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RING_BUFFER_SIZE 512

typedef struct {
    uint8_t buffer[RING_BUFFER_SIZE];
    volatile uint32_t head;
    volatile uint32_t tail;
    volatile uint32_t count;
} RingBuffer_t;

/** Empty the buffer and clear its storage. */
void RingBuffer_Init(RingBuffer_t *ringBuffer);

/** Append one byte; false when the buffer is full. */
bool RingBuffer_Put(RingBuffer_t *ringBuffer, uint8_t data);

/** Remove the oldest byte; false when the buffer is empty. */
bool RingBuffer_Get(RingBuffer_t *ringBuffer, uint8_t *data);

/**
 * @brief  Append several bytes, stopping when the buffer is full
 * @retval Number of bytes actually stored
 */
uint32_t RingBuffer_PutBytes(RingBuffer_t *ringBuffer, const uint8_t *data, uint32_t count);

/**
 * @brief  Read several bytes, all or nothing
 * @note   Leaves the buffer untouched unless the full count is queued, so a
 *         partial packet survives until a later call can complete it.
 */
bool RingBuffer_GetBytes(RingBuffer_t *ringBuffer, uint8_t *data, uint32_t count);

/**
 * @brief  Read whatever is queued, up to @p maxCount bytes
 * @note   Unlike RingBuffer_GetBytes() this never refuses a short read, which
 *         is what a stream reader wants: take what has arrived so far.
 * @retval Number of bytes copied into @p data, 0 if the buffer was empty
 */
uint32_t RingBuffer_Read(RingBuffer_t *ringBuffer, uint8_t *data, uint32_t maxCount);

/** Number of bytes currently queued. */
uint32_t RingBuffer_Available(const RingBuffer_t *ringBuffer);

/** True when the ring has reached its capacity. */
bool RingBuffer_IsFull(const RingBuffer_t *ringBuffer);

/** True when no bytes are queued. */
bool RingBuffer_IsEmpty(const RingBuffer_t *ringBuffer);

#ifdef __cplusplus
}
#endif

#endif /* UART_RING_BUFFER_H */
