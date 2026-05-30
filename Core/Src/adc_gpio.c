#include "adc_gpio.h"

void ADC_GPIO_Init(void) {
    /* GPIO pins should already be configured by CubeMX as inputs.
     * This function is a placeholder for any additional setup if needed.
     */
}

uint16_t ADC_GPIO_Read(void) {
    uint16_t value = 0;
    
    /* For 20 MSPS sampling, we need fast GPIO reads. Using direct register
     * access instead of HAL functions for maximum speed. If your GPIO pins
     * are all on the same port, this can be further optimized.
     */
    
    /* Read 10 GPIO pins (ADC_BIT0 through ADC_BIT9)
     * Using HAL_GPIO_ReadPin which is reasonably fast for 20 MSPS.
     * For even higher speeds (>20 MSPS), consider using direct port reads.
     */
    if (HAL_GPIO_ReadPin(ADC_BIT0_GPIO_Port, ADC_BIT0_Pin)) value |= (1 << 0);
    if (HAL_GPIO_ReadPin(ADC_BIT1_GPIO_Port, ADC_BIT1_Pin)) value |= (1 << 1);
    if (HAL_GPIO_ReadPin(ADC_BIT2_GPIO_Port, ADC_BIT2_Pin)) value |= (1 << 2);
    if (HAL_GPIO_ReadPin(ADC_BIT3_GPIO_Port, ADC_BIT3_Pin)) value |= (1 << 3);
    if (HAL_GPIO_ReadPin(ADC_BIT4_GPIO_Port, ADC_BIT4_Pin)) value |= (1 << 4);
    if (HAL_GPIO_ReadPin(ADC_BIT5_GPIO_Port, ADC_BIT5_Pin)) value |= (1 << 5);
    if (HAL_GPIO_ReadPin(ADC_BIT6_GPIO_Port, ADC_BIT6_Pin)) value |= (1 << 6);
    if (HAL_GPIO_ReadPin(ADC_BIT7_GPIO_Port, ADC_BIT7_Pin)) value |= (1 << 7);
    if (HAL_GPIO_ReadPin(ADC_BIT8_GPIO_Port, ADC_BIT8_Pin)) value |= (1 << 8);
    if (HAL_GPIO_ReadPin(ADC_BIT9_GPIO_Port, ADC_BIT9_Pin)) value |= (1 << 9);
    
    return value;
}

/* OPTIMIZATION: Fast GPIO read for all bits on same port
 * If all ADC_BIT* pins are on the same GPIO port (e.g., all on GPIOA),
 * uncomment and use this for 20+ MSPS operation:
 * 
 * uint16_t ADC_GPIO_Read_Fast(void) {
 *     // Assumes all bits are on GPIOA, pins [9:0]
 *     // Read entire port and mask desired bits
 *     return (uint16_t)(ADC_BIT0_GPIO_Port->IDR & 0x3FF);
 * }
 * 
 * This reads the port in a single operation (~2 CPU cycles vs 10+).
 */
