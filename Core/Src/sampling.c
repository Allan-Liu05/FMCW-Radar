#include "sampling.h"
#include "adc_gpio.h"
#include "arm_math.h"

static SampleBuffer sample_buffer = {0};
static uint32_t current_sample_rate = 0;

void Sampling_Init(uint32_t sample_rate_hz) {
    current_sample_rate = sample_rate_hz;
    sample_buffer.write_index = 0;
    sample_buffer.buffer_ready = false;
    
    /* Configure the timer for sampling interrupt.
     * This is a placeholder - actual timer setup should be done via CubeMX.
     * Timer period = SystemCoreClock / (prescaler * sample_rate_hz)
     */
}

void Sampling_Start(void) {
    sample_buffer.write_index = 0;
    sample_buffer.buffer_ready = false;
    
    /* Enable timer interrupt - implementation depends on which timer is used */
    /* Typically: HAL_TIM_Base_Start_IT(&htimX); */
}

void Sampling_Stop(void) {
    /* Disable timer interrupt */
    /* Typically: HAL_TIM_Base_Stop_IT(&htimX); */
}

SampleBuffer* Sampling_GetBuffer(void) {
    return &sample_buffer;
}

void Sampling_ClearBufferReady(void) {
    sample_buffer.buffer_ready = false;
    sample_buffer.write_index = 0;
}

void Sampling_TimerCallback(void) {
    /* Read ADC GPIO pins */
    uint16_t adc_value = ADC_GPIO_Read();
    
    /* Convert to float32 (0-1023 range) */
    sample_buffer.samples[sample_buffer.write_index] = (float32_t)adc_value;
    
    sample_buffer.write_index++;
    
    /* Check if buffer is full */
    if (sample_buffer.write_index >= SAMPLE_BUFFER_SIZE) {
        sample_buffer.buffer_ready = true;
        sample_buffer.write_index = 0;
    }
}
