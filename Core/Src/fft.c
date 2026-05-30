#include "fft.h"
#include <math.h>

static arm_rfft_fast_instance_f32 fft_instance;
static float32_t fft_input[FFT_SIZE];
static float32_t fft_output[FFT_SIZE];
static float32_t hann_window[FFT_SIZE];
static uint32_t sample_rate_hz;

static void ApplyHannWindow(float32_t* input, float32_t* windowed) {
    for (uint16_t i = 0; i < FFT_SIZE; i++) {
        windowed[i] = input[i] * hann_window[i];
    }
}

static void GenerateHannWindow(void) {
    for (uint16_t i = 0; i < FFT_SIZE; i++) {
        hann_window[i] = 0.5f * (1.0f - arm_cos_f32(2.0f * PI * i / (FFT_SIZE - 1)));
    }
}

void FFT_Init(uint32_t sample_rate_hz_param) {
    sample_rate_hz = sample_rate_hz_param;
    
    /* Initialize CMSIS-DSP FFT */
    arm_rfft_fast_init_f32(&fft_instance, FFT_SIZE);
    
    /* Generate Hann window coefficients */
    GenerateHannWindow();
}

void FFT_Process(float32_t* input, FFTResult* output) {
    /* Apply Hann window to input */
    ApplyHannWindow(input, fft_input);
    
    /* Perform real FFT */
    arm_rfft_fast_f32(&fft_instance, fft_input, fft_output, 0);
    
    /* Extract magnitude and phase from complex FFT output
     * FFT output format: [real0, real(N/2), real1, imag1, real2, imag2, ...]
     */
    
    output->sample_rate_hz = sample_rate_hz;
    
    /* DC component (bin 0) */
    output->magnitude[0] = fabsf(fft_output[0]) / FFT_SIZE;
    output->phase[0] = 0.0f;
    
    /* Nyquist frequency (bin N/2) */
    output->magnitude[FFT_SIZE / 2 - 1] = fabsf(fft_output[1]) / FFT_SIZE;
    output->phase[FFT_SIZE / 2 - 1] = 0.0f;
    
    /* Complex bins 1 to N/2-1 */
    for (uint16_t k = 1; k < FFT_SIZE / 2 - 1; k++) {
        float32_t real = fft_output[2 * k];
        float32_t imag = fft_output[2 * k + 1];
        
        /* Magnitude (normalized) */
        output->magnitude[k] = 2.0f * sqrtf(real * real + imag * imag) / FFT_SIZE;
        
        /* Phase in radians */
        output->phase[k] = atan2f(imag, real);
    }
}

float32_t FFT_GetBinFrequency(uint16_t bin_index) {
    if (bin_index >= FFT_SIZE / 2) {
        return 0.0f;
    }
    return (float32_t)bin_index * sample_rate_hz / FFT_SIZE;
}

float32_t FFT_GetPeakFrequency(FFTResult* result) {
    uint16_t peak_bin = 0;
    float32_t peak_magnitude = 0.0f;
    
    /* Skip DC and Nyquist components for peak detection */
    for (uint16_t k = 1; k < FFT_SIZE / 2 - 1; k++) {
        if (result->magnitude[k] > peak_magnitude) {
            peak_magnitude = result->magnitude[k];
            peak_bin = k;
        }
    }
    
    return FFT_GetBinFrequency(peak_bin);
}

float32_t FFT_GetMagnitudeAtFrequency(FFTResult* result, float32_t frequency_hz) {
    /* Find the closest bin to the requested frequency */
    float32_t bin_f = frequency_hz * FFT_SIZE / result->sample_rate_hz;
    uint16_t bin_index = (uint16_t)(bin_f + 0.5f);
    
    if (bin_index >= FFT_SIZE / 2) {
        return 0.0f;
    }
    
    return result->magnitude[bin_index];
}
