#ifndef SAMPLING_H
#define SAMPLING_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

#define SAMPLE_BUFFER_SIZE 1024

typedef struct {
    float32_t samples[SAMPLE_BUFFER_SIZE];
    uint16_t write_index;
    bool buffer_ready;
} SampleBuffer;

/**
 * @brief Initialize sampling system with timer interrupt
 * 
 * Sets up a timer to trigger ADC sampling at a constant rate.
 * @param sample_rate_hz Desired sampling frequency in Hz
 */
void Sampling_Init(uint32_t sample_rate_hz);

/**
 * @brief Start sampling
 * 
 * Enables the sampling timer and begins collecting samples.
 */
void Sampling_Start(void);

/**
 * @brief Stop sampling
 * 
 * Disables the sampling timer and stops collecting samples.
 */
void Sampling_Stop(void);

/**
 * @brief Get pointer to the current sample buffer
 * 
 * @return Pointer to SampleBuffer structure containing samples and status
 */
SampleBuffer* Sampling_GetBuffer(void);

/**
 * @brief Reset buffer ready flag after processing
 * 
 * Call this after you've processed a full buffer to allow new data collection.
 */
void Sampling_ClearBufferReady(void);

/**
 * @brief Sampling timer callback (called from timer ISR)
 * 
 * This function should be called from the hardware timer interrupt handler.
 * It reads the ADC GPIO pins and stores the sample.
 */
void Sampling_TimerCallback(void);

#endif /* SAMPLING_H */
