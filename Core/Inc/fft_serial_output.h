/**
 * @file fft_serial_output.h
 * @brief Serial transmission of FFT results to visualization server
 * 
 * This module handles serializing FFT data and transmitting it over UART
 * to the data visualization web server.
 * 
 * Packet Format (4104 bytes total):
 *   - Header (2 bytes): 0xAA 0xBB
 *   - Magnitude array (2048 bytes): 512 float32 values
 *   - Phase array (2048 bytes): 512 float32 values
 *   - Sample rate (4 bytes): uint32_t
 *   - Checksum (2 bytes): CRC16
 */

#ifndef FFT_SERIAL_OUTPUT_H
#define FFT_SERIAL_OUTPUT_H

#include "fft.h"
#include "main.h"

/**
 * @brief Initialize serial output for FFT data
 * 
 * Configure UART2 for 115200 baud output. Must be called in main()
 * after HAL_Init() and peripheral initialization.
 */
void FFT_SerialOutput_Init(void);

/**
 * @brief Send FFT result over serial link
 * 
 * Formats FFT data as a packet and transmits it to the visualization server.
 * Non-blocking transmission using interrupt-based UART.
 * 
 * @param result Pointer to FFTResult structure from FFT_Process()
 */
void FFT_SerialOutput_SendResult(FFTResult* result);

/**
 * @brief Get transmission status
 * 
 * @return 1 if previous transmission is complete and ready to send, 0 otherwise
 */
uint8_t FFT_SerialOutput_IsReady(void);

#endif /* FFT_SERIAL_OUTPUT_H */
