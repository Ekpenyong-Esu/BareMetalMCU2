/**
 * @file uart_ring_buffer.h
 * @brief Fixed-capacity byte queue (lock-free, single-producer/single-consumer)
 *
 * A plain container: it stores bytes and knows nothing about UART, so it has
 * no opinion on where the bytes came from and does no logging. The driver
 * embeds one per link so reception can outrun the reader.
 *
 * Design:
 * - Lock-free ring buffer using head/tail/count indices
 * - volatile indices for safe access from ISR (producer) and main loop (consumer)
 * - Power-of-two size (512) enables fast modulo via bitmask (but we use %)
 * - All operations are O(1) and non-blocking
 * - GetBytes is all-or-nothing: leaves buffer untouched unless full count available
 *   This preserves packet boundaries for variable-length frames
 *
 * Thread Safety:
 * - Single producer (ISR/DMA callback) + single consumer (main loop) = safe
 * - No mutexes needed; volatile ensures compiler doesn't optimize away checks
 * - Count is updated atomically (32-bit on Cortex-M4)
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
    volatile uint32_t head;   /*!< Write index (producer) */
    volatile uint32_t tail;   /*!< Read index (consumer) */
    volatile uint32_t count;  /*!< Bytes currently in buffer */
} RingBuffer_t;

/**
 * @brief  Initialize (empty) the ring buffer
 *
 * Clears the storage array and resets head/tail/count to 0.
 *
 * @param  ringBuffer Ring buffer to initialize
 */
void RingBuffer_Init(RingBuffer_t *ringBuffer);

/**
 * @brief  Append one byte
 *
 * @param  ringBuffer Ring buffer
 * @param  data       Byte to append
 * @retval true if stored, false if buffer is full
 */
bool RingBuffer_Put(RingBuffer_t *ringBuffer, uint8_t data);

/**
 * @brief  Remove the oldest byte
 *
 * @param  ringBuffer Ring buffer
 * @param  data       Out: oldest byte
 * @retval true if byte removed, false if buffer is empty
 */
bool RingBuffer_Get(RingBuffer_t *ringBuffer, uint8_t *data);

/**
 * @brief  Append several bytes, stopping when the buffer is full
 *
 * Copies as many bytes as fit. Does not block.
 *
 * @param  ringBuffer Ring buffer
 * @param  data       Source buffer
 * @param  count      Number of bytes to append
 * @retval Number of bytes actually stored (0..count)
 */
uint32_t RingBuffer_PutBytes(RingBuffer_t *ringBuffer, const uint8_t *data, uint32_t count);

/**
 * @brief  Read several bytes, all or nothing
 *
 * Only removes bytes if the full @p count is available. This preserves
 * packet boundaries: a partial frame stays in the buffer until a later
 * call can read it completely.
 *
 * @param  ringBuffer Ring buffer
 * @param  data       Destination buffer
 * @param  count      Number of bytes to read
 * @retval true if @p count bytes were read, false if insufficient data
 */
bool RingBuffer_GetBytes(RingBuffer_t *ringBuffer, uint8_t *data, uint32_t count);

/**
 * @brief  Get number of bytes available to read
 * @param  ringBuffer Ring buffer
 * @retval Bytes in buffer (0..RING_BUFFER_SIZE)
 */
uint32_t RingBuffer_Available(const RingBuffer_t *ringBuffer);

/**
 * @brief  Check if buffer is full
 * @param  ringBuffer Ring buffer
 * @retval true if no space for another byte
 */
bool RingBuffer_IsFull(const RingBuffer_t *ringBuffer);

/**
 * @brief  Check if buffer is empty
 * @param  ringBuffer Ring buffer
 * @retval true if no bytes available
 */
bool RingBuffer_IsEmpty(const RingBuffer_t *ringBuffer);

#ifdef __cplusplus
}
#endif

#endif /* UART_RING_BUFFER_H */
