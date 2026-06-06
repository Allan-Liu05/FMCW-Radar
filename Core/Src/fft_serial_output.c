/**
 * @file fft_serial_output.c
 * @brief Serial transmission of FFT results to visualization server
 */

#include "fft_serial_output.h"
#include <string.h>
#include <stdint.h>

/* Assuming UART2 handle is available in main.c as huart2 */
extern UART_HandleTypeDef huart2;

/* Packet buffer (4104 bytes) */
static uint8_t tx_buffer[4104];
static uint8_t tx_in_progress = 0;

/**
 * @brief Calculate CRC16 checksum
 * 
 * Polynomial: 0xA001 (reflected CRC-16-CCITT)
 * This matches the Python implementation in the visualization server.
 */
static uint16_t CRC16_Calculate(uint8_t* data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return crc;
}

void FFT_SerialOutput_Init(void) {
    /* UART2 should already be initialized in CubeMX with 115200 baud */
    /* If not, configure it here */
    tx_in_progress = 0;
}

void FFT_SerialOutput_SendResult(FFTResult* result) {
    if (tx_in_progress) {
        return;  /* Previous transmission still in progress */
    }
    
    uint16_t offset = 0;
    
    /* Header */
    tx_buffer[offset++] = 0xAA;
    tx_buffer[offset++] = 0xBB;
    
    /* Magnitude array (512 float32 values = 2048 bytes) */
    for (uint16_t i = 0; i < 512; i++) {
        uint32_t val_u32 = *(uint32_t*)&result->magnitude[i];
        tx_buffer[offset++] = (uint8_t)(val_u32 & 0xFF);
        tx_buffer[offset++] = (uint8_t)((val_u32 >> 8) & 0xFF);
        tx_buffer[offset++] = (uint8_t)((val_u32 >> 16) & 0xFF);
        tx_buffer[offset++] = (uint8_t)((val_u32 >> 24) & 0xFF);
    }
    
    /* Phase array (512 float32 values = 2048 bytes) */
    for (uint16_t i = 0; i < 512; i++) {
        uint32_t val_u32 = *(uint32_t*)&result->phase[i];
        tx_buffer[offset++] = (uint8_t)(val_u32 & 0xFF);
        tx_buffer[offset++] = (uint8_t)((val_u32 >> 8) & 0xFF);
        tx_buffer[offset++] = (uint8_t)((val_u32 >> 16) & 0xFF);
        tx_buffer[offset++] = (uint8_t)((val_u32 >> 24) & 0xFF);
    }
    
    /* Sample rate (4 bytes, little-endian) */
    tx_buffer[offset++] = (uint8_t)(result->sample_rate_hz & 0xFF);
    tx_buffer[offset++] = (uint8_t)((result->sample_rate_hz >> 8) & 0xFF);
    tx_buffer[offset++] = (uint8_t)((result->sample_rate_hz >> 16) & 0xFF);
    tx_buffer[offset++] = (uint8_t)((result->sample_rate_hz >> 24) & 0xFF);
    
    /* Calculate CRC16 checksum on data (excluding checksum field itself) */
    uint16_t crc = CRC16_Calculate(tx_buffer, 4102);
    tx_buffer[offset++] = (uint8_t)(crc & 0xFF);
    tx_buffer[offset++] = (uint8_t)((crc >> 8) & 0xFF);
    
    /* Transmit via UART (interrupt-based, non-blocking) */
    tx_in_progress = 1;
    HAL_UART_Transmit_IT(&huart2, tx_buffer, 4104);
}

uint8_t FFT_SerialOutput_IsReady(void) {
    return !tx_in_progress;
}

/**
 * @brief UART transmission complete callback
 * 
 * This function should be called from the UART2 TX interrupt handler.
 * It's typically called automatically by the HAL when using HAL_UART_Transmit_IT().
 * 
 * In STM32CubeMX, this is usually done via:
 * void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) in main.c
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == huart2.Instance) {
        tx_in_progress = 0;
    }
}
