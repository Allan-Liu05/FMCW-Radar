/**
 * FFT Implementation for FMCW Radar - Usage Guide
 * 
 * OVERVIEW
 * ========
 * This implementation provides a complete FFT processing pipeline for the FMCW radar
 * using a 10-bit external ADC connected via GPIO pins.
 * 
 * COMPONENTS
 * ==========
 * 
 * 1. adc_gpio.h/c
 *    - Reads 10-bit parallel ADC data from 10 GPIO pins
 *    - Requires GPIO pins to be named ADC_BIT0 through ADC_BIT9 in main.h
 *    - ADC_GPIO_Read() returns 16-bit value (bits 15:10 are zero-padded)
 * 
 * 2. sampling.h/c
 *    - Manages sample collection via timer interrupt
 *    - Configurable sample rate (default: 20 MSPS for ~10MHz Nyquist)
 *    - Ping-pong buffering with 1024-sample buffers
 *    - Sampling_TimerCallback() should be called from timer ISR
 * 
 * 3. fft.h/c
 *    - Implements 1024-point real FFT using CMSIS-DSP
 *    - Automatic Hann windowing for spectral leakage reduction
 *    - Outputs magnitude and phase for each frequency bin
 *    - Frequency bin mapping (DC to 10 MHz)
 * 
 * INITIALIZATION
 * ==============
 * In main.c (already done):
 * 
 *   ADC_GPIO_Init();           // Initialize GPIO pins for ADC input
 *   Sampling_Init(20000000);   // Initialize sampling at 20 MSPS
 *   FFT_Init(20000000);        // Initialize FFT with same sample rate
 *   Sampling_Start();          // Begin collecting samples
 * 
 * SAMPLING RATE SELECTION
 * =======================
 * The sample rate determines:
 * - Nyquist frequency: Fs / 2 (max detectable frequency)
 * - Frequency resolution: Fs / FFT_SIZE
 * 
 * Current setup:
 *   Fs = 20 MHz → Nyquist = 10 MHz (covers full signal bandwidth)
 *   Resolution = 20MHz / 1024 ≈ 19.5 kHz per bin
 * 
 * To change sample rate:
 * 1. Modify SAMPLE_RATE_HZ in main.c
 * 2. Update timer period: Period = SystemCoreClock / SAMPLE_RATE_HZ
 *    - For 20 MHz rate at 240 MHz clock: Period = 12
 *    - For 20 MHz rate at 480 MHz clock: Period = 24
 * 3. Rebuild and deploy
 * 
 * CONFIGURABLE FFT SIZE
 * =====================
 * Current: 1024-point FFT (defined in fft.h)
 * 
 * To change:
 * 1. Edit fft.h: Change #define FFT_SIZE (must be power of 2)
 * 2. Edit sampling.h: Change #define SAMPLE_BUFFER_SIZE to match FFT_SIZE
 * 3. Rebuild
 * 
 * Trade-offs:
 *   Larger FFT → Better frequency resolution, slower computation
 *   Smaller FFT → Faster results, coarser frequency bins
 * 
 * PROCESSING LOOP
 * ===============
 * The main loop checks if a buffer of samples is ready, then processes it:
 * 
 *   while (1) {
 *     SampleBuffer* buffer = Sampling_GetBuffer();
 *     
 *     if (buffer->buffer_ready) {
 *       FFT_Process(buffer->samples, &fft_result);
 *       
 *       // Use results
 *       float32_t peak_freq = FFT_GetPeakFrequency(&fft_result);
 *       float32_t mag_at_freq = FFT_GetMagnitudeAtFrequency(&fft_result, 100000.0f);
 *       
 *       Sampling_ClearBufferReady();
 *     }
 *   }
 * 
 * FREQUENCY MAPPING
 * =================
 * FFT bin k corresponds to frequency: f_k = k * Fs / FFT_SIZE
 * 
 * Example frequencies for 20MHz sample rate, 1024-point FFT:
 *   Bin 0   → 0 Hz (DC)
 *   Bin 256 → 5 MHz
 *   Bin 512 → 10 MHz (Nyquist)
 * 
 * API FUNCTIONS
 * =============
 * 
 * FFTResult fft_result;
 * FFT_Init(sample_rate_hz);
 * FFT_Process(sample_buffer, &fft_result);
 * 
 * float32_t freq = FFT_GetBinFrequency(bin_index);
 * float32_t peak = FFT_GetPeakFrequency(&fft_result);
 * float32_t mag = FFT_GetMagnitudeAtFrequency(&fft_result, 100000.0f);
 * 
 * OUTPUT INTERPRETATION
 * =====================
 * FFTResult contains:
 * - magnitude[FFT_SIZE/2]: Magnitude spectrum (0 to Nyquist frequency)
 * - phase[FFT_SIZE/2]: Phase spectrum in radians
 * - sample_rate_hz: Sample rate used for frequency mapping
 * 
 * Magnitude is normalized by FFT size, so peak value depends on input amplitude.
 * Use relative magnitudes for peak detection and thresholding.
 * 
 * REQUIRED CONFIGURATION
 * ======================
 * In CubeMX (FMCW_Radar.ioc):
 * 1. Configure 10 GPIO pins as inputs (e.g., PA0-PA9 or any available)
 * 2. Name them ADC_BIT0 through ADC_BIT9 in the GPIO naming dialog
 * 3. Configure a timer (e.g., TIM2) for sampling interrupt
 * 4. Call Sampling_TimerCallback() from the timer ISR
 * 
 * NOTES
 * =====
 * - The external ADC clock must be stable for accurate FFT results
 * - Ensure timer sample rate is consistent (no jitter)
 * - For best spectral quality, use signals without DC offset
 * - Peak detection works by finding maximum magnitude bin
 * 
 * FUTURE ENHANCEMENTS
 * ====================
 * - Add chirp z-transform for zoomed FFT analysis
 * - Implement averaging over multiple FFT buffers
 * - Add detection of multiple peaks with threshold
 * - Implement adaptive windowing based on signal type
 * - Add zero-padding for sub-bin frequency estimation
 */
