#ifndef ADC_GPIO_H
#define ADC_GPIO_H

#include "main.h"
#include <stdint.h>

/**
 * @brief Initialize GPIO pins for ADC data input
 * 
 * Configures 10 GPIO pins as inputs to read parallel ADC data.
 * Assumes pins are pre-configured by CubeMX or HAL initialization.
 */
void ADC_GPIO_Init(void);

/**
 * @brief Read 10-bit ADC value from GPIO pins
 * 
 * Reads the state of 10 GPIO pins and combines them into a single value.
 * The GPIO pins should be named ADC_BIT0 through ADC_BIT9 in main.h
 * 
 * @return 16-bit unsigned value (bits 15:10 are zero-padded)
 */
uint16_t ADC_GPIO_Read(void);

#endif /* ADC_GPIO_H */
