#ifndef FFT_H
#define FFT_H

#include "arm_math.h"
#include <stdint.h>

#define FFT_SIZE 1024

typedef struct {
    float32_t magnitude[FFT_SIZE / 2];
    float32_t phase[FFT_SIZE / 2];
    uint32_t sample_rate_hz;
} FFTResult;

/**
 * @brief Initialize FFT processing
 * 
 * Sets up the CMSIS-DSP FFT structures and windowing coefficients.
 * @param sample_rate_hz Sample rate in Hz (affects frequency mapping)
 */
void FFT_Init(uint32_t sample_rate_hz);

/**
 * @brief Process a buffer of samples with FFT
 * 
 * Applies Hann window, performs 1024-point FFT, and computes magnitude/phase.
 * @param input Array of float32_t samples (must be FFT_SIZE length)
 * @param output Pointer to FFTResult structure to store results
 */
void FFT_Process(float32_t* input, FFTResult* output);

/**
 * @brief Get frequency for a given FFT bin
 * 
 * Maps FFT bin index to actual frequency in Hz.
 * @param bin_index Bin number (0 to FFT_SIZE/2 - 1)
 * @return Frequency in Hz
 */
float32_t FFT_GetBinFrequency(uint16_t bin_index);

/**
 * @brief Find peak frequency in FFT result
 * 
 * Identifies the bin with maximum magnitude and returns corresponding frequency.
 * @param result Pointer to FFTResult from FFT_Process()
 * @return Peak frequency in Hz
 */
float32_t FFT_GetPeakFrequency(FFTResult* result);

/**
 * @brief Get magnitude at a specific frequency
 * 
 * Finds the closest bin to the requested frequency and returns magnitude.
 * @param result Pointer to FFTResult from FFT_Process()
 * @param frequency_hz Target frequency in Hz
 * @return Magnitude at that frequency
 */
float32_t FFT_GetMagnitudeAtFrequency(FFTResult* result, float32_t frequency_hz);

#endif /* FFT_H */
