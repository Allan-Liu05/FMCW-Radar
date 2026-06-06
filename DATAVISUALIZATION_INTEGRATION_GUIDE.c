/**
 * INTEGRATION GUIDE: How to add FFT serial output to your main.c
 * 
 * This file shows the exact changes needed to transmit FFT results
 * over serial to the data visualization web server.
 * 
 * Search for "// *** NEW" comments to find all required changes.
 */

/* USER CODE BEGIN Includes */
#include "adc_gpio.h"
#include "sampling.h"
#include "fft.h"
#include "fft_serial_output.h"  // *** NEW: Add this include
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
#define SAMPLE_RATE_HZ 20000000  /* 20 MSPS for ~10 MHz Nyquist frequency */

static FFTResult fft_result;
/* USER CODE END PV */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration*/
  MPU_Config();

  /* MCU Configuration*/
  HAL_Init();

  /* USER CODE BEGIN Init */
  ADC_GPIO_Init();
  Sampling_Init(SAMPLE_RATE_HZ);
  FFT_Init(SAMPLE_RATE_HZ);
  FFT_SerialOutput_Init();  // *** NEW: Initialize serial output
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
  Sampling_Start();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    SampleBuffer* buffer = Sampling_GetBuffer();
    
    /* Check if a full buffer of samples is ready */
    if (buffer->buffer_ready) {
      /* Process FFT on the collected samples */
      FFT_Process(buffer->samples, &fft_result);
      
      /* Get peak frequency */
      float32_t peak_freq = FFT_GetPeakFrequency(&fft_result);
      
      /* Example: Get magnitude at 100kHz */
      float32_t mag_100k = FFT_GetMagnitudeAtFrequency(&fft_result, 100000.0f);
      
      /* *** NEW: Send FFT result over serial to visualization server */
      if (FFT_SerialOutput_IsReady()) {
        FFT_SerialOutput_SendResult(&fft_result);
      }
      
      /* Clear the buffer ready flag to collect new samples */
      Sampling_ClearBufferReady();
      
      /* TODO: Use fft_result for further processing or output */
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  /* ... existing code ... */
}

/* *** NEW: Add this callback function for UART transmission completion */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  /* This will be called automatically when UART transmission completes */
  /* The fft_serial_output.c module handles the callback internally */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/***
 * SUMMARY OF CHANGES
 * ==================
 * 
 * 1. Include "fft_serial_output.h" in the Includes section
 * 2. Call FFT_SerialOutput_Init() after FFT_Init() in main()
 * 3. Call FFT_SerialOutput_SendResult(&fft_result) in the main loop
 *    after FFT_Process() completes
 * 4. Optionally: Add HAL_UART_TxCpltCallback() (it's handled by fft_serial_output.c)
 * 
 * CONFIGURATION
 * =============
 * 
 * In STM32CubeMX:
 * - Ensure UART2 is configured for 115200 baud (or your chosen UART)
 * - Enable UART TX interrupt
 * - Enable global interrupt for the UART
 * 
 * In datavisualization_app.py:
 * - Set SERIAL_PORT to match your STM32's COM port (COM3, COM4, etc.)
 * - Set SERIAL_BAUD to 115200 (or your chosen baud rate)
 * 
 * BUILD & RUN
 * ===========
 * 
 * 1. Add fft_serial_output.c and fft_serial_output.h to your project
 * 2. Make the changes above to main.c
 * 3. Build the firmware
 * 4. Flash to STM32
 * 5. Run: python datavisualization_app.py
 * 6. Open: http://localhost:5000
 */
