# Quick Start Guide - FFT Data Visualization

## 5-Minute Setup

### Prerequisites
- Python 3.7+ installed
- STM32 microcontroller with FFT firmware
- USB serial connection to STM32

### Step 1: Install Python Dependencies (1 min)

```bash
cd c:\Waterloo\Projects\FMCW\FMCW_Radar
pip install -r datavisualization_requirements.txt
```

### Step 2: Update Firmware (5 min)

1. **Open main.c** in `Core\Src\main.c`

2. **Add include** at the top of Private Includes section:
   ```c
   #include "fft_serial_output.h"
   ```

3. **Add initialization** in `main()` after `MX_GPIO_Init()`:
   ```c
   FFT_SerialOutput_Init();
   Sampling_Start();
   ```

4. **Add data transmission** in the main loop after `FFT_Process()`:
   ```c
   while (1) {
       SampleBuffer* buffer = Sampling_GetBuffer();
       
       if (buffer->buffer_ready) {
           FFT_Process(buffer->samples, &fft_result);
           
           // NEW: Send FFT result over serial
           if (FFT_SerialOutput_IsReady()) {
               FFT_SerialOutput_SendResult(&fft_result);
           }
           
           Sampling_ClearBufferReady();
       }
   }
   ```

5. **Build and Flash** the firmware

### Step 3: Find Your Serial Port

**Windows:**
- Open Device Manager
- Look under "Ports (COM & LPT)"
- Note the COM port (e.g., COM3, COM4)

**Linux/Mac:**
```bash
# Linux
ls /dev/ttyUSB*

# Mac  
ls /dev/tty.usbserial*
```

### Step 4: Configure Web Server

Edit `datavisualization_app.py`, line ~23:
```python
SERIAL_PORT = 'COM3'  # ← Change to your port
SERIAL_BAUD = 115200
```

### Step 5: Start Web Server

```bash
python datavisualization_app.py
```

You should see:
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

### Step 6: Open Web Browser

Navigate to: **http://localhost:5000**

You should see the FFT plot updating in real-time!

## Verify Everything Works

✓ **Web page loads** - You can see the frequency domain plot  
✓ **Connection indicator** - Shows "Connected" in green  
✓ **Packet counter** - Increasing number of packets received  
✓ **Plot updates** - Real-time magnitude spectrum displayed  
✓ **Peak information** - Shows detected peak frequency  

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Port not found | Check Device Manager, verify STM32 is connected |
| "Failed to open serial port" | Close other programs using the COM port |
| Web page won't load | Check Python server is running (you should see the banner) |
| Connection shows "Disconnected" | Verify baud rate is 115200 on both firmware and app.py |
| No data in plot | Ensure FFT_SerialOutput_SendResult() is being called |

## Features at a Glance

- **Real-time plot**: Interactive Plotly.js visualization
- **Peak detection**: Automatic frequency peak identification
- **Statistics**: Track packets received and errors
- **Export**: Download FFT data as CSV
- **Responsive**: Works on desktop and mobile browsers

## Next Steps

1. **Customize plot settings**: Edit styling in `datavisualization_app.py`
2. **Change baud rate**: Update `SERIAL_BAUD` for faster updates (115200→921600)
3. **Add averaging**: Modify firmware to average multiple FFT results
4. **Log data**: Extend server to save FFT results to database

## File Reference

- `datavisualization_app.py` - Main Flask web server
- `Core/Inc/fft_serial_output.h` - Serial output header (add to firmware)
- `Core/Src/fft_serial_output.c` - Serial output implementation (add to firmware)
- `DATAVISUALIZATION_README.md` - Full documentation

---

**Questions?** Check `DATAVISUALIZATION_README.md` for detailed documentation
