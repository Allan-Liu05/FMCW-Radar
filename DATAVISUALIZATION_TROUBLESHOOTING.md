# Data Visualization - Troubleshooting Checklist

## Pre-Setup Checklist

- [ ] Python 3.7+ installed and working
- [ ] STM32 microcontroller connected via USB
- [ ] Serial port driver installed (check Device Manager)
- [ ] No other application using the serial port
- [ ] Text editor or IDE ready to modify main.c
- [ ] CMakeLists.txt accessible

## Installation Verification

### Python Dependencies
```bash
# Run this command
pip install -r datavisualization_requirements.txt

# Verify installation
python -c "import flask; import serial; print('OK')"
```

- [ ] No error messages from pip install
- [ ] Dependencies import successfully

### Firmware Files
```bash
# Verify files exist
ls Core/Inc/fft_serial_output.h
ls Core/Src/fft_serial_output.c
```

- [ ] Both .h and .c files present
- [ ] Files have correct permissions (readable)

### Web Server Startup
```bash
# Run web server
python datavisualization_app.py
```

- [ ] Server starts without errors
- [ ] Shows banner with configuration
- [ ] Listening on http://localhost:5000
- [ ] No "port already in use" error

## Firmware Integration Verification

### main.c Changes
In `Core/Src/main.c`:

- [ ] `#include "fft_serial_output.h"` present
- [ ] `FFT_SerialOutput_Init()` called after `MX_GPIO_Init()`
- [ ] `FFT_SerialOutput_SendResult(&fft_result)` called in main loop
- [ ] Placed after `FFT_Process()` call
- [ ] Inside the `if (buffer->buffer_ready)` block

### CMakeLists.txt Changes
In `CMakeLists.txt`:

- [ ] `Core/Src/fft_serial_output.c` added to SOURCES list
- [ ] Correct spelling and path
- [ ] Proper comma placement

### Build Verification
```bash
mkdir build
cd build
cmake -G "Unix Makefiles" ..
make
```

- [ ] No compilation errors
- [ ] No linker errors
- [ ] `.elf` file generated
- [ ] No "undefined reference to FFT_SerialOutput" errors

### Flash Verification
- [ ] Firmware flashes successfully to STM32
- [ ] No verification errors
- [ ] Reset button works after flash

## UART Configuration Verification

In STM32CubeMX (FMCW_Radar.ioc):

- [ ] UART2 is enabled
- [ ] Baud rate set to 115200
- [ ] Mode: Asynchronous
- [ ] TX interrupt enabled
- [ ] NVIC settings: UART2 global interrupt enabled
- [ ] Pin assignment: TX and RX connected to used pins

## Serial Port Verification

### Find Your Port

**Windows (Command Prompt):**
```cmd
wmic logicaldisk get name
mode
```
Look for COM port numbers

- [ ] Serial port found in Device Manager
- [ ] Correct COM number noted (COM3, COM4, etc.)
- [ ] No conflicts/errors shown in Device Manager

**Linux/Mac:**
```bash
ls /dev/tty*
```

- [ ] Serial port device found
- [ ] Correct path noted (/dev/ttyUSB0, etc.)
- [ ] User has read/write permissions

### Connection Test
```python
# Quick test script
import serial

try:
    port = serial.Serial('COM3', 115200, timeout=1)  # Change COM3 to your port
    print("Port opened successfully")
    port.close()
except Exception as e:
    print(f"Error: {e}")
```

- [ ] Port opens without error
- [ ] No "Access Denied" message
- [ ] No "Port not found" error

## Web Interface Verification

### Browser Access
- [ ] Open `http://localhost:5000` in web browser
- [ ] Page loads (not "Cannot connect" error)
- [ ] See "FMCW Radar - FFT Visualization" header
- [ ] See empty plot area with axes

### Connection Status
In browser, look for status bar:

- [ ] Green circle with "Connected" (if firmware sending data)
- [ ] Red circle with "Disconnected" (expected if firmware not running)
- [ ] Packet count showing (usually 0 if not connected)

### API Endpoints
Test in browser address bar:

```
http://localhost:5000/api/connection_status
http://localhost:5000/api/stats
http://localhost:5000/api/fft_data
```

- [ ] Each returns JSON (not HTML error)
- [ ] No 404 errors
- [ ] Data fields present in JSON

## Real-Time Data Verification

Once STM32 firmware is running:

### Check Connection Status
- [ ] Status indicator shows "Connected" (green)
- [ ] Port shows correct COM number
- [ ] Baud rate shows 115200

### Check Packet Reception
- [ ] Packet counter is increasing (updates every 500ms)
- [ ] Error count stays at 0 or low numbers
- [ ] "Last Update" time changes recently

### Check Plot Display
- [ ] Plot area shows frequency domain data
- [ ] X-axis labeled "Frequency (MHz)"
- [ ] Y-axis labeled "Magnitude"
- [ ] Plot shows smooth curve (not jagged)
- [ ] Plot updates in real-time (watch it change)

### Check Peak Information
- [ ] Peak Frequency shows a number (not "-")
- [ ] Peak Magnitude shows a number (not "-")
- [ ] Frequency is in MHz (reasonable for FMCW)

## Troubleshooting by Symptom

### Web Page Won't Load
```
Symptom: "Cannot connect to localhost:5000" or timeout
```

**Checklist:**
- [ ] Flask server is running (check terminal)
- [ ] No other service using port 5000
- [ ] Firewall not blocking localhost
- [ ] Browser cache cleared (Ctrl+F5)
- [ ] Try different browser (Chrome/Firefox/Edge)

**Fix:**
```bash
# Kill any process using port 5000
netstat -ano | findstr :5000  # Windows
lsof -i :5000                 # Linux/Mac

# Restart server
python datavisualization_app.py
```

### Status Shows "Disconnected"
```
Symptom: Red circle, "Disconnected", packet count at 0
```

**Checklist:**
- [ ] STM32 is powered on
- [ ] USB cable connected and working
- [ ] Serial port in app matches actual port
- [ ] Baud rate is 115200
- [ ] UART2 initialized in firmware
- [ ] Firmware is calling FFT_SerialOutput_SendResult()

**Fix:**
```bash
# In datavisualization_app.py, change SERIAL_PORT:
SERIAL_PORT = 'COM4'  # Try different COM numbers

# Or auto-detect:
import serial.tools.list_ports
ports = serial.tools.list_ports.comports()
for port in ports:
    print(port.device)
```

### No Data in Plot
```
Symptom: Connected but plot is empty, stays empty
```

**Checklist:**
- [ ] Packet counter is increasing
- [ ] Error count is 0 or very low
- [ ] FFT is being processed (add LED blink to verify)
- [ ] FFT_SerialOutput_SendResult() is being called
- [ ] UART TX interrupt is working

**Fix:**
1. Verify firmware is running:
   - Add LED toggle in FFT processing loop
   - Watch LED blink at expected rate
2. Verify UART transmission:
   - Check UART interrupt is firing
   - Monitor serial data with terminal program (PuTTY, TeraTerm)
3. Check packet format:
   - Verify header bytes (0xAA 0xBB) are correct
   - Check CRC16 calculation matches

### Plot Updates Slowly or Stops
```
Symptom: Plot updates infrequently or freezes
```

**Checklist:**
- [ ] Packet counter still increasing
- [ ] No high error rate
- [ ] CPU usage reasonable (<50%)
- [ ] RAM usage not excessive
- [ ] UART buffer not overflowing

**Fix:**
```bash
# In datavisualization_app.py, increase refresh rate:
# Change update interval from 500ms to 1000ms (slower)
setInterval(updatePlot, 1000);  # In JavaScript
```

### Serial Port Access Denied
```
Symptom: "Access Denied" or "Permission Denied"
```

**Windows:**
- [ ] Close other programs using COM port
- [ ] Run Python as Administrator
- [ ] Check Device Manager for COM port conflicts

**Linux/Mac:**
- [ ] Check permissions: `ls -la /dev/ttyUSB0`
- [ ] Add user to dialout group: `sudo usermod -a -G dialout $USER`
- [ ] Run with sudo: `sudo python datavisualization_app.py`

### High Error Rate (Checksum Failures)
```
Symptom: "Checksum mismatch" messages, packet errors increasing
```

**Checklist:**
- [ ] Serial cable is good (try different cable)
- [ ] Baud rate mismatch between STM32 and app
- [ ] Electrical noise (RF interference)
- [ ] Buffer overflow in STM32

**Fix:**
1. Slow down serial rate (try 57600 instead of 115200)
2. Add delay between FFT results in firmware
3. Use higher quality/shorter serial cable
4. Reduce electromagnetic interference

## Performance Diagnostics

### Check Update Frequency
In browser console (F12):
```javascript
// Monitor API response times
fetch('/api/fft_data').then(r => r.json()).then(d => console.log(Date.now(), d));
```

- [ ] Responses faster than 100ms
- [ ] No timeouts or slow responses

### Check Network Tab
In browser (F12 → Network):

- [ ] `fft_data` request: <100ms
- [ ] `stats` request: <50ms
- [ ] Plot rendering: <200ms
- [ ] No failed requests (404/500 errors)

### Check Server Logs
Terminal running Python server:

- [ ] No error messages
- [ ] Serial reader thread running
- [ ] Successful packet parsing
- [ ] Normal CRC16 checksum values

## Reset & Restart

If having persistent issues:

### Full Reset Procedure
```bash
# 1. Stop web server
Ctrl+C

# 2. Close browser (clear cache)
# Ctrl+Shift+Delete to clear cache

# 3. Disconnect STM32

# 4. Verify files again
ls Core/Inc/fft_serial_output.h
ls Core/Src/fft_serial_output.c

# 5. Restart server
python datavisualization_app.py

# 6. Reconnect STM32

# 7. Reopen browser to http://localhost:5000
```

- [ ] Server starts cleanly
- [ ] Web page loads
- [ ] Data begins flowing

## Getting Help

If still not working:

1. **Check README**: `DATAVISUALIZATION_README.md`
2. **Review logs**: Look at Python server terminal output
3. **Monitor serial**: Use third-party serial monitor to verify data
4. **Inspect network**: Use browser DevTools (F12)
5. **Verify firmware**: Confirm FFT is processing with indicator LED

## Success Indicators

✅ System is working when you see:
- Green "Connected" status in web UI
- Packet count increasing (1-2 per second at typical rates)
- Real-time plot updating smoothly
- Peak frequency value changing appropriately
- No error messages in server logs
- CSV export works

---

**Still stuck?** Double-check each section in order. Most issues are COM port misconfiguration or missing file integration.
