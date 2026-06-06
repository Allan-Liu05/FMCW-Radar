# Data Visualization System - Implementation Summary

## What Has Been Created

A complete real-time FFT data visualization system for the FMCW Radar project with three main components:

### 1. **Web Server** (`datavisualization_app.py`)
- Flask-based Python web application running on `localhost:5000`
- Reads FFT data from STM32 over serial (115200 baud)
- Serves interactive Plotly.js-based frequency domain plot
- REST API endpoints for FFT data access
- Real-time packet statistics and connection monitoring
- Background thread for non-blocking serial I/O
- CRC16 checksum validation for data integrity

**Features:**
- Real-time magnitude spectrum visualization
- Automatic peak frequency detection
- Live connection status indicator
- Packet reception/error statistics
- CSV export functionality
- Responsive web UI with dark theme

### 2. **Firmware Integration Files**
   
**`Core/Inc/fft_serial_output.h`**
- Header file defining serial output API
- Function prototypes: `FFT_SerialOutput_Init()`, `FFT_SerialOutput_SendResult()`, `FFT_SerialOutput_IsReady()`
- Packet format documentation

**`Core/Src/fft_serial_output.c`**
- Implementation of serial FFT data transmission
- Packet formatting (4104 bytes with header, magnitude, phase, sample rate, CRC16)
- CRC16 checksum calculation
- Non-blocking interrupt-based UART transmission
- HAL callback for transmission completion

### 3. **Documentation**

**`DATAVISUALIZATION_QUICKSTART.md`** (5-minute setup guide)
- Step-by-step integration instructions
- Serial port detection for Windows/Linux/Mac
- Troubleshooting quick reference
- Configuration changes needed

**`DATAVISUALIZATION_README.md`** (Complete documentation)
- Architecture overview and system design
- Detailed component descriptions
- REST API specification
- Firmware integration guide
- Serial packet format specification
- Performance considerations
- Advanced usage examples
- Troubleshooting guide

**`DATAVISUALIZATION_INTEGRATION_GUIDE.c`** (Code example)
- Annotated main.c showing exact integration points
- Comments marking all required changes
- Summary of modifications needed
- Configuration checklist

**`CMAKELISTS_UPDATE_GUIDE.txt`**
- Instructions for updating CMakeLists.txt
- How to add new source files to build

## System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                                                                 │
│  STM32H7 Microcontroller                                       │
│  ├─ ADC GPIO Input (10-bit parallel)                          │
│  ├─ Sampling (Timer interrupt, 20 MSPS)                       │
│  ├─ FFT Processing (1024-point CMSIS-DSP)                     │
│  └─ FFT Serial Output (UART2, 115200 baud)                    │
│     └─ Non-blocking UART TX (4104 bytes per result)           │
│                                                                 │
├─ USB Serial Cable (FTDI/STLink Virtual COM)                   │
│                                                                 │
├─ Flask Web Server (Python)                                     │
│  ├─ Serial Reader Thread                                       │
│  │  ├─ Read raw bytes from COM port                           │
│  │  ├─ Parse 4104-byte packets                                │
│  │  ├─ Verify CRC16 checksum                                  │
│  │  ├─ Extract magnitude, phase, frequencies                  │
│  │  └─ Update global FFT data buffer                          │
│  │                                                             │
│  ├─ REST API Endpoints                                        │
│  │  ├─ GET / → HTML+CSS+JS web interface                     │
│  │  ├─ GET /api/fft_data → FFT magnitude/phase/freq (JSON)   │
│  │  ├─ GET /api/stats → Packet stats                         │
│  │  ├─ GET /api/connection_status → Port/baud info           │
│  │  └─ GET /api/peak_info → Peak freq & magnitude            │
│  │                                                             │
│  └─ HTTP Server (Port 5000)                                   │
│                                                                 │
├─ Web Browser                                                    │
│  ├─ HTML + CSS for layout and styling                         │
│  ├─ Plotly.js for interactive plotting                        │
│  ├─ JavaScript for AJAX updates (500ms refresh)               │
│  ├─ Real-time frequency domain plot                           │
│  ├─ Connection status indicator                               │
│  ├─ Peak frequency/magnitude display                          │
│  ├─ Packet statistics                                         │
│  └─ Export button for CSV data download                       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## Serial Packet Format

```
Offset  Length  Field           Type        Bytes
─────────────────────────────────────────────────
0       2       Header          uint8_t[2]  0xAA 0xBB
2       2048    Magnitude       float32[512] 512 magnitudes
2050    2048    Phase           float32[512] 512 phases
4098    4       Sample Rate     uint32_t    20,000,000 Hz
4102    2       Checksum        uint16_t    CRC16 of bytes 0-4101
─────────────────────────────────────────────
Total:  4104 bytes per FFT result
```

## Integration Steps

### Step 1: Add Files to Project
- Copy `Core/Inc/fft_serial_output.h` to your Core/Inc directory
- Copy `Core/Src/fft_serial_output.c` to your Core/Src directory

### Step 2: Update CMakeLists.txt
- Add `Core/Src/fft_serial_output.c` to the SOURCES list

### Step 3: Modify main.c
1. Add include: `#include "fft_serial_output.h"`
2. Call `FFT_SerialOutput_Init()` after other initializations
3. Call `FFT_SerialOutput_SendResult(&fft_result)` after `FFT_Process()`

### Step 4: Configure UART2 in CubeMX
- Ensure UART2 is configured for 115200 baud
- Enable UART TX interrupt
- Enable global interrupt

### Step 5: Build & Flash
- Rebuild firmware with new files
- Flash to STM32

### Step 6: Run Web Server
```bash
pip install -r datavisualization_requirements.txt
python datavisualization_app.py
```

### Step 7: Open Web Interface
- Navigate to `http://localhost:5000` in web browser

## File Locations

```
FMCW_Radar/
├── datavisualization_app.py          ← Main Flask web server
├── datavisualization_requirements.txt ← Python dependencies
├── DATAVISUALIZATION_README.md        ← Full documentation
├── DATAVISUALIZATION_QUICKSTART.md    ← 5-min setup guide
├── DATAVISUALIZATION_INTEGRATION_GUIDE.c ← Code integration example
├── CMAKELISTS_UPDATE_GUIDE.txt       ← Build system guide
│
└── Core/
    ├── Inc/
    │   ├── fft_serial_output.h       ← NEW: Serial output header
    │   └── fft.h                      ← Existing FFT header
    │
    └── Src/
        ├── fft_serial_output.c       ← NEW: Serial output implementation
        ├── fft.c                      ← Existing FFT implementation
        └── main.c                     ← MODIFY: Add serial transmission calls
```

## Configuration Reference

### Python App (`datavisualization_app.py`)
```python
SERIAL_PORT = 'COM3'           # Change to your serial port
SERIAL_BAUD = 115200           # Baud rate (match firmware)
FFT_SIZE = 1024                # FFT points
SAMPLE_RATE_HZ = 20000000      # Sample rate in Hz
NUM_BINS = FFT_SIZE // 2       # Output bins (512)
```

### Web Server
- **URL**: `http://localhost:5000`
- **Plot Update Rate**: 500ms (edit in JavaScript)
- **Serial Timeout**: 1.0s per read operation
- **CRC16 Polynomial**: 0xA001 (reflected)

### Firmware
- **UART**: UART2
- **Baud Rate**: 115200
- **Packet Size**: 4104 bytes
- **FFT Configuration**: 1024-point, 20 MSPS

## Next Steps

1. **Read the Quick Start Guide**: `DATAVISUALIZATION_QUICKSTART.md`
2. **Add files to your project**: Copy .h and .c files to Core directories
3. **Update main.c**: Add three lines (include, init, send)
4. **Update CMakeLists.txt**: Add new source file
5. **Build and flash**: Compile and deploy firmware
6. **Run web server**: Execute `python datavisualization_app.py`
7. **Open browser**: Navigate to `http://localhost:5000`

## Support Resources

- **Quick Start**: `DATAVISUALIZATION_QUICKSTART.md` (5 minutes)
- **Full Documentation**: `DATAVISUALIZATION_README.md` (comprehensive)
- **Code Example**: `DATAVISUALIZATION_INTEGRATION_GUIDE.c` (annotated)
- **Build Guide**: `CMAKELISTS_UPDATE_GUIDE.txt` (CMake setup)
- **Troubleshooting**: See README for common issues

## Verification Checklist

After setup, verify:
- [ ] Firmware compiles without errors
- [ ] Firmware flashes successfully to STM32
- [ ] Web server starts: `python datavisualization_app.py`
- [ ] Web page loads: `http://localhost:5000`
- [ ] Connection status shows "Connected" (green)
- [ ] Packet counter is increasing
- [ ] FFT plot is updating in real-time
- [ ] Peak frequency is displayed
- [ ] CSV export works

## Performance Notes

- **Update Rate**: ~2 Hz (4104 bytes @ 115200 baud ≈ 360ms per packet)
- **Web Refresh**: 500ms (can be adjusted)
- **Memory Usage**: ~4 MB total (STM32 TX buffer + Python + Plotly)
- **CPU Load**: Low (Flask runs in separate thread)

---

**Ready to start?** Begin with `DATAVISUALIZATION_QUICKSTART.md` for step-by-step instructions.

All files are documented and ready for integration!
