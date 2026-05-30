# FFT Implementation - Integration Checklist

Complete these steps to finish integrating the FFT system with your hardware.

## ✅ Code Integration (DONE)

- [x] ADC GPIO driver created
- [x] Sampling control module created
- [x] FFT processing module created
- [x] main.c updated with FFT initialization and processing loop
- [x] CMakeLists.txt updated with CMSIS-DSP and new source files
- [x] Documentation created

## 🔧 Hardware Configuration (ACTION REQUIRED)

### Step 1: GPIO Pin Configuration
- [ ] Open `FMCW_Radar.ioc` in STM32CubeMX
- [ ] Configure 10 GPIO pins as digital inputs
- [ ] **Assign these exact names in CubeMX**:
  - ADC_BIT0 through ADC_BIT9
- [ ] These must match the code in `Core/Src/adc_gpio.c` line 13-22
- [ ] **Performance tip**: If all 10 bits are on the same port (PA[9:0], PB[9:0], etc.),
  uncomment the fast read function in `adc_gpio.c` for optimal 20 MSPS sampling
- [ ] Regenerate code (Project → Generate Code)

### Step 2: Timer Configuration
- [ ] Select a timer (TIM2, TIM3, TIM5, etc.)
- [ ] Configure for periodic interrupt mode
- [ ] Set timer period to achieve 20 MSPS sampling:
  - **Period** = SystemCoreClock / 20,000,000
  - For 240 MHz clock: Period = 12
  - For 480 MHz clock: Period = 24
  - Adjust for your actual system clock
- [ ] Enable timer interrupt
- [ ] Regenerate code

### Step 3: Add Timer Interrupt Handler
Edit `Core/Src/stm32h7xx_it.c` and add this code:

```c
// Add extern declaration at top of file
extern void Sampling_TimerCallback(void);

// Find HAL_TIM_PeriodElapsedCallback() or create if it doesn't exist
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {  // Change TIM2 to your selected timer
        Sampling_TimerCallback();
    }
}
```

**Important**: The timer interrupt MUST call `Sampling_TimerCallback()` for sampling to work.

### Step 4: Enable Timer in main.c
- [ ] The timer should be started in CubeMX/HAL init
- [ ] OR add `HAL_TIM_Base_Start_IT(&htim2);` in main before Sampling_Start()
  - Replace `htim2` with your timer handle name

## 🏗️ Build Instructions

```bash
cd C:\Waterloo\Projects\FMCW\FMCW_Radar

# Configure build (with Debug or Release preset)
cmake --preset Debug -B build

# Build
cmake --build build

# Clean build if needed
cmake --build build --target clean
cmake --build build
```

## 📊 Verification Checklist

After building and deploying:

- [ ] Code compiles without errors
- [ ] No undefined reference errors for GPIO pins
- [ ] Board boots and runs main loop
- [ ] FFT processing begins (watch Sampling_GetBuffer()->buffer_ready flag)
- [ ] FFT results contain non-zero magnitude values
- [ ] Peak frequency calculation works (check with test signal)

## 📝 Test Suggestions

### Manual Testing with Debugger
1. Set breakpoint in main loop after `FFT_Process()`
2. Connect external ADC and apply 1 MHz test signal
3. Check `fft_result.magnitude[k]` values
4. Verify peak appears near expected frequency bin

### Automated Testing
1. Inject known frequency sine wave to ADC
2. Read FFT peak frequency
3. Verify it matches injected frequency (±20 kHz tolerance)

## 🔍 Troubleshooting

**Build Errors: "undefined reference to ADC_BIT0_GPIO_Port"**
- [ ] Check GPIO names in `adc_gpio.c` match CubeMX exactly
- [ ] Regenerate code from CubeMX
- [ ] Verify `main.h` contains GPIO definitions

**Sampling stops after a few buffers**
- [ ] Verify timer ISR is actually running (toggle LED in callback)
- [ ] Check timer clock is enabled in CubeMX
- [ ] Ensure no stack overflow (check HAL init)

**FFT output is all zeros**
- [ ] Verify GPIO pins are receiving ADC data (debug read values)
- [ ] Check Hann window isn't being multiplied twice
- [ ] Verify input samples are in expected range (0-1023)

**Peak detection always returns 0 Hz**
- [ ] Check sample rate matches SAMPLE_RATE_HZ in main.c
- [ ] Verify FFT is actually being called (add debug output)
- [ ] Ensure FFT output magnitude is computed correctly

## 📚 Reference

- **FFT API Docs**: See `Core/FFT_README.md`
- **Usage Examples**: See `Core/Inc/FFT_USAGE.h` comments
- **Configuration**: Edit `SAMPLE_RATE_HZ` in `main.c` or `FFT_SIZE` in `fft.h`

## 🎯 Expected Performance

Once configured:
- **Sampling latency**: ~51 µs per 1024 samples @ 20 MSPS
- **FFT latency**: ~5-10 ms per transform
- **Total latency**: ~10-15 ms per FFT result
- **Memory overhead**: ~8 KB buffers + FFT structures

## ✨ Next Steps After Integration

Once hardware integration is complete:

1. Test with various signal frequencies to verify mapping
2. Implement peak frequency detection thresholding
3. Add averaging over multiple FFT buffers for noise reduction
4. Consider overlap-add for continuous analysis
5. Optimize FFT window if needed (Hamming, Blackman, etc.)

---

**Questions?** Refer to detailed docs in `Core/FFT_README.md` or code comments in individual modules.
