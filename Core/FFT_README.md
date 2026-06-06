# FFT Implementation for FMCW Radar

This directory contains a complete FFT processing pipeline for the FMCW radar system, designed to process signals from a 10-bit external ADC connected via GPIO pins.

## Quick Start

The FFT system is already integrated into `main.c`. To use it:

1. **Configure GPIO pins in CubeMX** (FMCW_Radar.ioc):
   - Create 10 GPIO input pins for the ADC bits
   - Name them: `ADC_BIT0`, `ADC_BIT1`, ..., `ADC_BIT9`
   - Configure a timer (e.g., TIM2) for sampling interrupt

2. **Add timer ISR callback** in `stm32h7xx_it.c`:
   ```c
   extern void Sampling_TimerCallback(void);
   
   void TIM2_IRQHandler(void) {
       HAL_TIM_IRQHandler(&htim2);
   }
   
   void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
       if (htim->Instance == TIM2) {
           Sampling_TimerCallback();
       }
   }
   ```

3. **Build and run** - FFT processing starts automatically in the main loop

## System Architecture

### Data Flow
```
GPIO Pins (10-bit ADC) 
    ↓
ADC_GPIO_Read() 
    ↓
Sampling Buffer (1024 samples @ Fs)
    ↓
FFT_Process() with Hann Window
    ↓
Magnitude & Phase Spectrum
    ↓
Frequency Analysis
```

### Modules

| Module | Purpose |
|--------|---------|
| `adc_gpio.h/c` | Read 10-bit parallel ADC data from GPIO pins |
| `sampling.h/c` | Timer-driven sample collection and buffering |
| `fft.h/c` | 1024-point FFT with CMSIS-DSP, windowing |

## Key Parameters

| Parameter | Value | Notes |
|-----------|-------|-------|
| FFT Size | 1024 | Configurable, must be power of 2 |
| Sample Rate | 20 MSPS | Covers 10 MHz to Nyquist frequency |
| Nyquist Frequency | 10 MHz | Max detectable frequency |
| Frequency Resolution | ~19.5 kHz | Resolution per bin |
| Sample Buffer | 1024 samples | Automatically managed |

## API Reference

### ADC GPIO
```c
void ADC_GPIO_Init(void);           // Initialize GPIO pins
uint16_t ADC_GPIO_Read(void);       // Read 10-bit value (0-1023)
```

### Sampling
```c
void Sampling_Init(uint32_t sample_rate_hz);
void Sampling_Start(void);
void Sampling_Stop(void);
SampleBuffer* Sampling_GetBuffer(void);
void Sampling_ClearBufferReady(void);
void Sampling_TimerCallback(void);  // Call from timer ISR
```

### FFT Processing
```c
void FFT_Init(uint32_t sample_rate_hz);
void FFT_Process(float32_t* input, FFTResult* output);
float32_t FFT_GetBinFrequency(uint16_t bin_index);
float32_t FFT_GetPeakFrequency(FFTResult* result);
float32_t FFT_GetMagnitudeAtFrequency(FFTResult* result, float32_t frequency_hz);
```

### Data Structures
```c
typedef struct {
    float32_t magnitude[FFT_SIZE / 2];  // 512 bins: DC to Nyquist
    float32_t phase[FFT_SIZE / 2];      // Phase in radians
    uint32_t sample_rate_hz;
} FFTResult;

typedef struct {
    float32_t samples[SAMPLE_BUFFER_SIZE];
    uint16_t write_index;
    bool buffer_ready;
} SampleBuffer;
```

## Usage Example

```c
// In main loop
while (1) {
    SampleBuffer* buffer = Sampling_GetBuffer();
    
    if (buffer->buffer_ready) {
        // Perform FFT
        FFT_Process(buffer->samples, &fft_result);
        
        // Get peak frequency
        float32_t peak_freq = FFT_GetPeakFrequency(&fft_result);
        
        // Get magnitude at specific frequency
        float32_t mag_100k = FFT_GetMagnitudeAtFrequency(&fft_result, 100000.0f);
        
        // Get magnitude at bin 256 (512 kHz for 2MHz sample rate)
        float32_t mag_bin256 = fft_result.magnitude[256];
        
        // Clear buffer for next acquisition
        Sampling_ClearBufferReady();
    }
}
```

## Frequency Mapping

For a 20 MHz sample rate and 1024-point FFT:

| Bin | Frequency |
|-----|-----------|
| 0 | 0 Hz (DC) |
| 1 | 19.5 kHz |
| 256 | 5 MHz |
| 512 | 10 MHz (Nyquist) |

Formula: `frequency = bin_index * sample_rate / FFT_SIZE`

## Configuration

### Changing Sample Rate
1. Modify `SAMPLE_RATE_HZ` in `main.c`
2. Update timer prescaler/period in CubeMX to match new rate
3. For 20 MSPS: Timer period = SystemCoreClock / 20,000,000
   - At 240 MHz: Period = 12
   - At 480 MHz: Period = 24
4. Rebuild

### Changing FFT Size
1. Edit `fft.h`: Change `#define FFT_SIZE` (power of 2)
2. Edit `sampling.h`: Change `#define SAMPLE_BUFFER_SIZE` to match
3. Rebuild

Note: Larger FFTs provide better frequency resolution but take longer to compute.

## Performance

- **FFT Computation Time**: ~5-10 ms for 1024-point FFT on STM32H7
- **Sample Collection Time**: ~51 µs for 1024 samples @ 20 MSPS
- **Overall Latency**: ~10-15 ms per FFT result

## Integration Notes

1. **CMSIS-DSP Library**: Already linked in CMakeLists.txt (`-lm` for math functions)
2. **Memory Requirements**: ~8 KB for buffers and FFT structures
3. **Timer Setup**: Configure timer period for desired sample rate
4. **GPIO Configuration**: 10 GPIO pins required for parallel ADC

## Windowing

The implementation uses a **Hann window** to reduce spectral leakage:
- Automatically applied during FFT processing
- Provides good frequency resolution and leakage reduction
- Pre-computed for efficiency

## Future Enhancements

- [ ] Implement overlap-add for continuous analysis
- [ ] Add multiple FFT averaging for noise reduction
- [ ] Implement peak detection with threshold
- [ ] Add chirp z-transform for zoomed analysis
- [ ] Support for different window functions (Hamming, Blackman)

## Dependencies

- **CMSIS-DSP**: Included in STM32CubeH7 → `arm_math.h`
- **STM32 HAL**: For GPIO and timer access
- **C Math Library**: For trigonometric functions

## Troubleshooting

**Issue**: FFT output is all zeros
- Check GPIO pins are correctly configured and named in CubeMX
- Verify timer interrupt is triggering at expected rate
- Check ADC GPIO reads are returning valid values

**Issue**: Peak frequency incorrect
- Verify sample rate matches actual timer frequency
- Check GPIO bits are connected to correct ADC output lines
- Ensure stable external ADC clock

**Issue**: Build errors about undefined GPIO pins
- Ensure GPIO names in `adc_gpio.c` match CubeMX definitions
- Regenerate code from CubeMX if pins were renamed

## References

- ARM CMSIS-DSP Library: https://arm-software.github.io/CMSIS_5/DSP/html/index.html
- STM32H7 Reference Manual: STM32H7B0xx datasheet
- FFT Tutorial: https://en.wikipedia.org/wiki/Fast_Fourier_transform
