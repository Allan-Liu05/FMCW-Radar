# 🛰️ FMCW Radar - Data Visualization System

## What Is This?

A complete real-time web-based FFT visualization system for your FMCW Radar project. Displays frequency domain plots of radar signals by streaming FFT data from your STM32 microcontroller over a serial link to a Python web server running in your browser.

**In 30 seconds:**
1. Your firmware processes FFT ✓ (already done)
2. Firmware sends FFT results over serial ✓ (new - we created this)
3. Python web server reads serial data ✓ (new - we created this)
4. Your browser displays interactive plot ✓ (new - we created this)

## 🚀 Quick Start (5 Minutes)

1. **Read this**: `DATAVISUALIZATION_QUICKSTART.md`
2. **Copy files**: `.h` and `.c` files to Core directories
3. **Modify code**: Add 3 lines to main.c
4. **Update build**: Add file to CMakeLists.txt
5. **Run**: `python datavisualization_app.py`
6. **View**: Open `http://localhost:5000` in browser

## 📚 Documentation Map

### For Beginners
- **Start here**: `DATAVISUALIZATION_QUICKSTART.md` - 5-min setup guide
- **Having issues?**: `DATAVISUALIZATION_TROUBLESHOOTING.md` - Diagnostic checklist

### For Integration
- **How to add to main.c**: `DATAVISUALIZATION_INTEGRATION_GUIDE.c` - Annotated code example
- **Build system setup**: `CMAKELISTS_UPDATE_GUIDE.txt` - CMake modifications
- **What's installed**: `DATAVISUALIZATION_SUMMARY.md` - Full overview

### For Reference
- **Complete docs**: `DATAVISUALIZATION_README.md` - Comprehensive guide
- **Architecture**: See system diagram in README
- **API specification**: See REST endpoints in README

## 📁 Files Created

### Web Server (Python)
- `datavisualization_app.py` - Main Flask application
- `datavisualization_requirements.txt` - Python dependencies

### Firmware (C)
- `Core/Inc/fft_serial_output.h` - Header file
- `Core/Src/fft_serial_output.c` - Implementation

### Documentation
- `DATAVISUALIZATION_README.md` - Full documentation
- `DATAVISUALIZATION_QUICKSTART.md` - 5-minute setup
- `DATAVISUALIZATION_SUMMARY.md` - System overview
- `DATAVISUALIZATION_INTEGRATION_GUIDE.c` - Code example
- `DATAVISUALIZATION_TROUBLESHOOTING.md` - Diagnostics
- `CMAKELISTS_UPDATE_GUIDE.txt` - Build system guide

## ⚡ What It Does

### Firmware Side
- ✅ Processes FFT (already in place)
- ✅ Formats FFT data as 4104-byte packets
- ✅ Sends over UART2 @ 115200 baud
- ✅ Non-blocking (interrupt-based)
- ✅ CRC16 checksum for data integrity

### Server Side
- ✅ Listens on serial port (configurable COM port)
- ✅ Parses 4104-byte packets
- ✅ Verifies checksum
- ✅ Serves REST API with FFT data
- ✅ Hosts web interface

### Browser Side
- ✅ Real-time Plotly.js plot
- ✅ Shows frequency domain magnitude spectrum
- ✅ Updates every 500ms
- ✅ Detects and displays peak frequency
- ✅ Shows connection status
- ✅ Tracks packet statistics
- ✅ Exports data as CSV

## 🔧 Integration Checklist

### Before You Start
- [ ] Python 3.7+ installed
- [ ] STM32 connected via USB
- [ ] Serial port driver installed
- [ ] Text editor ready to modify main.c

### Installation
- [ ] Copy `fft_serial_output.h` to Core/Inc/
- [ ] Copy `fft_serial_output.c` to Core/Src/
- [ ] Update CMakeLists.txt (add new source file)
- [ ] Modify main.c (3 lines: include, init, send)

### Build & Deploy
- [ ] Firmware compiles without errors
- [ ] Firmware flashes successfully
- [ ] STM32 boots normally

### Run Web Server
- [ ] `pip install -r datavisualization_requirements.txt`
- [ ] `python datavisualization_app.py`
- [ ] Server starts and shows configuration
- [ ] Open `http://localhost:5000` in browser
- [ ] See FFT plot updating in real-time

## 🛠️ Configuration

### Change Serial Port
Edit `datavisualization_app.py`:
```python
SERIAL_PORT = 'COM3'  # Your COM port number
```

### Find Your Serial Port
**Windows**: Device Manager → Ports (COM & LPT)  
**Linux**: `ls /dev/ttyUSB*`  
**Mac**: `ls /dev/tty.usbserial*`

### Change Baud Rate
Both firmware and app must match:
```python
SERIAL_BAUD = 115200  # Or 57600, 460800, 921600
```

## 📊 System Diagram

```
┌──────────────────┐
│    STM32H7       │
│  Microcontroller │
├──────────────────┤
│ • ADC Input      │
│ • FFT Processing │
│ • Serial Output  │
│   (UART2,115200) │
└────────┬─────────┘
         │ USB Serial
         ▼
┌──────────────────┐
│   Python Flask   │
│   Web Server     │
├──────────────────┤
│ • Serial Reader  │
│ • Data Parser    │
│ • REST API       │
│ • HTTP Server    │
└────────┬─────────┘
         │ HTTP (port 5000)
         ▼
┌──────────────────┐
│   Web Browser    │
│  http://localhost │
│     :5000        │
├──────────────────┤
│ • Plotly.js Plot │
│ • Real-time Data │
│ • Status Display │
│ • CSV Export     │
└──────────────────┘
```

## 📈 Performance

- **Update rate**: ~2-3 Hz (depends on UART baud rate)
- **Web refresh**: 500ms (configurable)
- **Packet size**: 4104 bytes per FFT result
- **CPU load**: Low (background thread)
- **Memory**: ~4 MB total system

## 🔌 Serial Packet Format

```
Header (2B)    | Magnitude (2048B) | Phase (2048B) | Rate (4B) | CRC16 (2B)
0xAA 0xBB      | 512 floats        | 512 floats    | uint32_t  | CRC16
├─────────────────────────────────────────────────────────────────────────┤
                        Total: 4104 bytes per packet
```

## ✅ Success Indicators

You'll know it's working when:
1. Web page loads at `http://localhost:5000`
2. Status shows "Connected" (green indicator)
3. Packet counter is increasing
4. FFT plot shows real-time data
5. Peak frequency displays a value
6. Plot updates smoothly every 500ms

## ❓ Need Help?

### Quick Issues
- **Can't find COM port?** → See Windows Device Manager (Ports section)
- **"Failed to open serial port"?** → Wrong port or already in use
- **Web page won't load?** → Check Flask server is running
- **No data in plot?** → Verify firmware is sending data

### Documentation
- **Setup issues**: `DATAVISUALIZATION_QUICKSTART.md`
- **Integration issues**: `DATAVISUALIZATION_INTEGRATION_GUIDE.c`
- **Technical details**: `DATAVISUALIZATION_README.md`
- **All problems**: `DATAVISUALIZATION_TROUBLESHOOTING.md`

## 📝 File Reference

| File | Purpose |
|------|---------|
| `datavisualization_app.py` | Main web server (Python) |
| `datavisualization_requirements.txt` | Python package dependencies |
| `Core/Inc/fft_serial_output.h` | Serial output API header |
| `Core/Src/fft_serial_output.c` | Serial output implementation |
| `DATAVISUALIZATION_README.md` | Complete documentation |
| `DATAVISUALIZATION_QUICKSTART.md` | 5-minute setup guide |
| `DATAVISUALIZATION_SUMMARY.md` | System overview |
| `DATAVISUALIZATION_INTEGRATION_GUIDE.c` | Code integration example |
| `DATAVISUALIZATION_TROUBLESHOOTING.md` | Diagnostic checklist |
| `CMAKELISTS_UPDATE_GUIDE.txt` | CMake build system |

## 🎯 Next Steps

1. **Read**: `DATAVISUALIZATION_QUICKSTART.md`
2. **Copy**: Files to Core directories
3. **Modify**: main.c (follow DATAVISUALIZATION_INTEGRATION_GUIDE.c)
4. **Build**: Compile firmware
5. **Deploy**: Flash to STM32
6. **Run**: `python datavisualization_app.py`
7. **View**: `http://localhost:5000`

---

## Features Summary

✨ **Real-time Visualization**
- Interactive Plotly.js plot
- Frequency domain magnitude spectrum
- Live updates (500ms refresh)

🎯 **Peak Detection**
- Automatic peak frequency identification
- Peak magnitude display
- Frequency in Hz and MHz

📊 **Monitoring**
- Connection status indicator
- Packet reception counter
- Error tracking
- Last update timestamp

💾 **Data Export**
- CSV download of FFT data
- Frequencies, magnitudes, phases
- Timestamp information

🔧 **Developer Friendly**
- REST API endpoints
- Well-documented code
- Easy configuration
- Integration guides

---

**Ready to get started?** → Open `DATAVISUALIZATION_QUICKSTART.md`

**Need details?** → See `DATAVISUALIZATION_README.md`

**Having issues?** → Check `DATAVISUALIZATION_TROUBLESHOOTING.md`
