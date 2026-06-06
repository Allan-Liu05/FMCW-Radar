# FMCW Radar FFT Data Visualization

A real-time web-based frequency domain visualization system for the FMCW Radar project. Displays FFT magnitude and phase data transmitted from the STM32H7 microcontroller over a serial link.

## Overview

```
STM32H7 Microcontroller          Python Flask Server          Web Browser
     (FMCW Radar)                                              (Real-time Plot)
         |                              |                           |
         |-- FFT Processing             |                           |
         |-- Serializes FFT Data        |                           |
         |-- UART Transmission ------>  |-- Reads Serial Data       |
         |                              |-- REST API Endpoints      |
         |                              |-- Serves Web Interface -->|
         |                              |                           |
         |                              |-- Plotly.js Library      |
         |                              |-- Real-time Updates      |
```

## Features

- **Real-time Frequency Domain Plot**: Interactive Plotly.js visualization of FFT magnitude spectrum
- **Peak Detection**: Automatic identification and display of peak frequency and magnitude
- **Live Statistics**: Packet reception count, error tracking, connection status
- **Data Export**: Download FFT data as CSV for offline analysis
- **Responsive UI**: Mobile-friendly web interface with gradient styling
- **Non-blocking Serial I/O**: Interrupt-based UART transmission from STM32
- **CRC16 Checksum**: Data integrity verification for serial packets
- **Configurable**: Easy serial port and baud rate configuration

## Components

### 1. Firmware (STM32H7)

#### Files:
- `Core/Inc/fft_serial_output.h` - Header with serial output API
- `Core/Src/fft_serial_output.c` - Serial transmission implementation

#### Key Functions:
```c
void FFT_SerialOutput_Init(void);                    // Initialize UART
void FFT_SerialOutput_SendResult(FFTResult* result); // Send FFT data
uint8_t FFT_SerialOutput_IsReady(void);              // Check if ready to send
```

#### Serial Packet Format (4104 bytes):

| Offset | Length | Field          | Type      | Description                  |
|--------|--------|----------------|-----------|------------------------------|
| 0      | 2      | Header         | uint8_t   | 0xAA, 0xBB                   |
| 2      | 2048   | Magnitude      | float32[] | 512 magnitude values         |
| 2050   | 2048   | Phase          | float32[] | 512 phase values (radians)   |
| 4098   | 4      | Sample Rate    | uint32_t  | Sample rate in Hz            |
| 4102   | 2      | Checksum       | uint16_t  | CRC16 of first 4102 bytes    |

### 2. Web Server (Python Flask)

#### Setup:

```bash
# Install dependencies
pip install -r datavisualization_requirements.txt

# Run the server
python datavisualization_app.py
```

#### Server Configuration:

Edit `datavisualization_app.py` to change:
- `SERIAL_PORT`: COM3, COM4, /dev/ttyUSB0, etc.
- `SERIAL_BAUD`: Default 115200
- `FFT_SIZE`: FFT points (default 1024)
- `SAMPLE_RATE_HZ`: Sample rate in Hz

#### REST API Endpoints:

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/` | GET | Web interface (HTML + Plotly) |
| `/api/fft_data` | GET | Current FFT magnitude, phase, frequencies (JSON) |
| `/api/stats` | GET | Packet stats and connection status |
| `/api/connection_status` | GET | Serial port info and error count |
| `/api/peak_info` | GET | Peak frequency and magnitude |

### 3. Firmware Integration

#### Step 1: Include Header
In `main.c`, add:
```c
#include "fft_serial_output.h"
```

#### Step 2: Initialize
In `main()` after peripherals are initialized:
```c
FFT_SerialOutput_Init();  // After MX_GPIO_Init()
```

#### Step 3: Send Data
In the main loop after FFT processing:
```c
while (1) {
    SampleBuffer* buffer = Sampling_GetBuffer();
    
    if (buffer->buffer_ready) {
        FFT_Process(buffer->samples, &fft_result);
        
        /* Send FFT result over serial */
        if (FFT_SerialOutput_IsReady()) {
            FFT_SerialOutput_SendResult(&fft_result);
        }
        
        Sampling_ClearBufferReady();
    }
}
```

#### Step 4: UART Configuration
In STM32CubeMX:
1. Configure UART2 (or your chosen UART)
2. Set baud rate to 115200
3. Enable global interrupt for UART TX
4. Ensure `HAL_UART_TxCpltCallback()` is defined in main.c (already included in fft_serial_output.c)

## Quick Start

### 1. Build and Flash Firmware

```bash
# In FMCW_Radar project directory
mkdir build
cd build
cmake -G "Unix Makefiles" ..
make
# Flash to STM32 using your preferred method
```

### 2. Start Web Server

```bash
# From repository root
python datavisualization_app.py
```

Expected output:
```
============================================================
FMCW Radar FFT Visualization Server
============================================================
Web Interface: http://localhost:5000
Serial Port: COM3 @ 115200 baud
FFT Size: 1024 points
Sample Rate: 20 MSPS
============================================================
```

### 3. Open Web Browser

Navigate to: `http://localhost:5000`

You should see:
- Real-time frequency domain plot
- Live connection status indicator
- Peak frequency and magnitude values
- Packet reception statistics

## Configuration

### Change Serial Port

Edit `datavisualization_app.py`:
```python
SERIAL_PORT = 'COM3'  # Change to your port
SERIAL_BAUD = 115200  # Change baud rate if needed
```

### List Available Serial Ports (Python)

```python
import serial.tools.list_ports
ports = serial.tools.list_ports.comports()
for port in ports:
    print(f"{port.device}: {port.description}")
```

### Windows: Find Serial Port

Device Manager → Ports (COM & LPT) → Note the COM number

### Linux/Mac: Find Serial Port

```bash
# Linux
ls /dev/ttyUSB* /dev/ttyACM*

# Mac
ls /dev/tty.usbserial*
```

## Troubleshooting

### "Failed to open serial port"

1. Verify COM port is correct (Device Manager on Windows)
2. Ensure no other application is using the port
3. Check baud rate matches (115200)
4. Verify STM32 is plugged in and powered

### No data appearing in browser

1. Check web server is running: `python datavisualization_app.py`
2. Verify STM32 firmware includes `FFT_SerialOutput_SendResult()` calls
3. Check serial connection indicator in web UI
4. Monitor browser console (F12) for JavaScript errors
5. Verify UART2 is configured in STM32CubeMX

### Packet errors showing in statistics

1. Check serial cable connection quality
2. Verify baud rate is 115200 on both sides
3. Reduce baud rate if using long cables or noisy environment
4. Check for buffer overflows in STM32 (increase UART buffer if needed)

### Plot not updating

1. Confirm connection status shows "Connected"
2. Check packet count is increasing
3. Verify FFT is being processed (add LED blink in main loop)
4. Check browser's Network tab (F12) for API responses

## Performance Considerations

### Update Rate
- Default: 500ms refresh rate in web browser
- Adjustable in `datavisualization_app.py` (client-side)

### Serial Bandwidth
- 4104 bytes per FFT result
- @ 115200 baud: ~355 ms per packet
- Alternative: Use higher baud rate (460800, 921600) for faster updates

### Memory
- STM32 TX buffer: 4104 bytes (temporary)
- Python server: ~1 MB for FFT data structures
- Web browser: ~5 MB (includes Plotly library)

## Advanced Usage

### Multiple FFT Results Averaging

Modify main loop to average multiple FFT results:
```c
static uint16_t fft_count = 0;
static float32_t averaged_magnitude[512] = {0};

if (buffer->buffer_ready) {
    FFT_Process(buffer->samples, &fft_result);
    
    for (int i = 0; i < 512; i++) {
        averaged_magnitude[i] += fft_result.magnitude[i];
    }
    
    fft_count++;
    if (fft_count >= 10) {  // Average over 10 FFTs
        for (int i = 0; i < 512; i++) {
            fft_result.magnitude[i] = averaged_magnitude[i] / 10.0f;
        }
        
        FFT_SerialOutput_SendResult(&fft_result);
        fft_count = 0;
        memset(averaged_magnitude, 0, sizeof(averaged_magnitude));
    }
    
    Sampling_ClearBufferReady();
}
```

### Custom Frequency Ranges

The web interface can be zoomed to focus on specific frequency ranges using Plotly's zoom feature (drag to zoom, double-click to reset).

## Files Reference

```
datavisualization/
├── datavisualization_app.py           # Flask web server
├── datavisualization_requirements.txt  # Python dependencies
├── README.md                           # This file
└── Core/
    ├── Inc/fft_serial_output.h        # Serial output header
    └── Src/fft_serial_output.c        # Serial output implementation
```

## License

This component is part of the FMCW Radar project.

## Future Enhancements

- [ ] Multi-channel FFT support
- [ ] Real-time spectrogram (waterfall plot)
- [ ] Detection algorithm overlay
- [ ] Data logging to file
- [ ] Web dashboard with multiple radars
- [ ] WebSocket for persistent connection
- [ ] Frequency zoom and pan controls
- [ ] Magnitude threshold detection

## Support

For issues or questions:
1. Check the troubleshooting section
2. Review browser console for JavaScript errors
3. Check Python server logs for serial errors
4. Verify serial packet format matches specification

---

**Web Server Address**: http://localhost:5000  
**Default Serial Port**: COM3  
**Default Baud Rate**: 115200  
**FFT Configuration**: 1024-point, 20 MSPS, 512 output bins
