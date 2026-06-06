"""
FMCW Radar FFT Data Visualization Web Server

This Flask application:
- Reads FFT results from the STM32 microcontroller via serial port
- Serves a real-time frequency domain plot
- Provides REST API endpoints for FFT data access
"""

from flask import Flask, render_template_string, jsonify
import serial
import struct
import threading
import json
from datetime import datetime
import logging
import os

app = Flask(__name__)
app.config['JSON_SORT_KEYS'] = False

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Configuration
SERIAL_PORT = 'COM3'  # Change this to your serial port (COM3, COM4, /dev/ttyUSB0, etc.)
SERIAL_BAUD = 115200
FFT_SIZE = 1024
SAMPLE_RATE_HZ = 20000000  # 20 MSPS
NUM_BINS = FFT_SIZE // 2

# Data storage
fft_data = {
    'magnitude': [0.0] * NUM_BINS,
    'phase': [0.0] * NUM_BINS,
    'frequencies': [i * SAMPLE_RATE_HZ / FFT_SIZE for i in range(NUM_BINS)],
    'sample_rate': SAMPLE_RATE_HZ,
    'timestamp': None,
    'peak_frequency': 0.0,
    'peak_magnitude': 0.0
}

# Serial connection
serial_conn = None
serial_thread = None
stop_reading = False
data_lock = threading.Lock()

# Statistics
stats = {
    'packets_received': 0,
    'packets_errors': 0,
    'last_update': None,
    'is_connected': False
}


def open_serial_connection():
    """Open serial port connection to STM32."""
    global serial_conn
    try:
        serial_conn = serial.Serial(
            port=SERIAL_PORT,
            baudrate=SERIAL_BAUD,
            timeout=1.0,
            bytesize=serial.EIGHTBITS,
            stopbits=serial.STOPBITS_ONE,
            parity=serial.PARITY_NONE
        )
        logger.info(f"Serial connection opened: {SERIAL_PORT} @ {SERIAL_BAUD} baud")
        stats['is_connected'] = True
        return True
    except Exception as e:
        logger.error(f"Failed to open serial port {SERIAL_PORT}: {e}")
        stats['is_connected'] = False
        return False


def close_serial_connection():
    """Close serial port connection."""
    global serial_conn
    if serial_conn and serial_conn.is_open:
        serial_conn.close()
        logger.info("Serial connection closed")
        stats['is_connected'] = False


def parse_fft_packet(data):
    """
    Parse FFT data packet from STM32.
    Format:
    - Header (2 bytes): 0xAA 0xBB
    - Magnitude array (512 * 4 bytes): 512 float32 values
    - Phase array (512 * 4 bytes): 512 float32 values
    - Sample rate (4 bytes): uint32_t
    - Checksum (2 bytes): CRC16
    Total: 2 + 2048 + 2048 + 4 + 2 = 4104 bytes
    """
    try:
        if len(data) < 4104:
            logger.warning(f"Incomplete packet: {len(data)} bytes (expected 4104)")
            return None
        
        # Verify header
        if data[0] != 0xAA or data[1] != 0xBB:
            logger.warning(f"Invalid header: {hex(data[0])} {hex(data[1])}")
            return None
        
        # Parse magnitude array
        magnitude = []
        for i in range(NUM_BINS):
            offset = 2 + i * 4
            value = struct.unpack_from('<f', data, offset)[0]
            magnitude.append(value)
        
        # Parse phase array
        phase = []
        for i in range(NUM_BINS):
            offset = 2 + 2048 + i * 4
            value = struct.unpack_from('<f', data, offset)[0]
            phase.append(value)
        
        # Parse sample rate
        sample_rate = struct.unpack_from('<I', data, 2 + 2048 + 2048)[0]
        
        # Verify checksum (simple CRC16)
        checksum_received = struct.unpack_from('<H', data, 4102)[0]
        checksum_calc = calculate_crc16(data[:4102])
        
        if checksum_received != checksum_calc:
            logger.warning(f"Checksum mismatch: received {hex(checksum_received)}, calculated {hex(checksum_calc)}")
            return None
        
        return {
            'magnitude': magnitude,
            'phase': phase,
            'sample_rate': sample_rate
        }
    except Exception as e:
        logger.error(f"Error parsing FFT packet: {e}")
        return None


def calculate_crc16(data):
    """
    Calculate simple CRC16 checksum.
    This must match the firmware implementation.
    """
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc


def serial_reader_thread():
    """
    Background thread to continuously read from serial port
    and update FFT data.
    """
    global stop_reading, stats
    
    buffer = bytearray()
    
    while not stop_reading:
        try:
            if not serial_conn or not serial_conn.is_open:
                break
            
            # Read available bytes
            if serial_conn.in_waiting > 0:
                chunk = serial_conn.read(serial_conn.in_waiting)
                buffer.extend(chunk)
                
                # Look for complete packets (header + 4102 bytes)
                while len(buffer) >= 4104:
                    # Look for header
                    if buffer[0] == 0xAA and buffer[1] == 0xBB:
                        # Extract packet
                        packet = bytes(buffer[:4104])
                        result = parse_fft_packet(packet)
                        
                        if result:
                            # Update global FFT data
                            with data_lock:
                                fft_data['magnitude'] = result['magnitude']
                                fft_data['phase'] = result['phase']
                                fft_data['sample_rate'] = result['sample_rate']
                                fft_data['timestamp'] = datetime.now().isoformat()
                                
                                # Calculate peak frequency and magnitude
                                peak_idx = fft_data['magnitude'].index(max(fft_data['magnitude']))
                                fft_data['peak_magnitude'] = fft_data['magnitude'][peak_idx]
                                fft_data['peak_frequency'] = fft_data['frequencies'][peak_idx]
                                
                                stats['packets_received'] += 1
                        else:
                            stats['packets_errors'] += 1
                        
                        # Remove processed packet from buffer
                        buffer = buffer[4104:]
                    else:
                        # Skip one byte and look for next header
                        buffer.pop(0)
            else:
                # Small sleep to avoid busy waiting
                import time
                time.sleep(0.001)
        
        except Exception as e:
            logger.error(f"Serial reader thread error: {e}")
            stats['packets_errors'] += 1


def start_serial_reader():
    """Start the serial reader background thread."""
    global serial_thread, stop_reading
    if open_serial_connection():
        stop_reading = False
        serial_thread = threading.Thread(target=serial_reader_thread, daemon=True)
        serial_thread.start()
        logger.info("Serial reader thread started")


def stop_serial_reader():
    """Stop the serial reader background thread."""
    global stop_reading
    stop_reading = True
    if serial_thread:
        serial_thread.join(timeout=2)
    close_serial_connection()
    logger.info("Serial reader thread stopped")


# Flask Routes

@app.route('/')
def index():
    """Serve the main visualization page."""
    html = """
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>FMCW Radar - FFT Visualization</title>
        <script src="https://cdn.plot.ly/plotly-latest.min.js"></script>
        <style>
            * {
                margin: 0;
                padding: 0;
                box-sizing: border-box;
            }
            
            body {
                font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                background: linear-gradient(135deg, #1e3c72 0%, #2a5298 100%);
                color: #333;
                min-height: 100vh;
                padding: 20px;
            }
            
            .container {
                max-width: 1400px;
                margin: 0 auto;
            }
            
            header {
                background: rgba(255, 255, 255, 0.95);
                padding: 20px;
                border-radius: 10px;
                margin-bottom: 20px;
                box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            }
            
            h1 {
                color: #1e3c72;
                margin-bottom: 10px;
            }
            
            .status-bar {
                display: flex;
                gap: 20px;
                flex-wrap: wrap;
                align-items: center;
            }
            
            .status-item {
                display: flex;
                align-items: center;
                gap: 8px;
                padding: 10px 15px;
                background: #f5f5f5;
                border-radius: 5px;
                border-left: 4px solid #2a5298;
            }
            
            .status-indicator {
                width: 12px;
                height: 12px;
                border-radius: 50%;
                background: #ff6b6b;
            }
            
            .status-indicator.connected {
                background: #51cf66;
            }
            
            .main-content {
                display: grid;
                grid-template-columns: 1fr 300px;
                gap: 20px;
                margin-bottom: 20px;
            }
            
            @media (max-width: 1200px) {
                .main-content {
                    grid-template-columns: 1fr;
                }
            }
            
            .chart-container {
                background: rgba(255, 255, 255, 0.95);
                border-radius: 10px;
                padding: 20px;
                box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            }
            
            #plot {
                width: 100%;
                height: 500px;
            }
            
            .sidebar {
                background: rgba(255, 255, 255, 0.95);
                border-radius: 10px;
                padding: 20px;
                box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
                height: fit-content;
            }
            
            .info-section {
                margin-bottom: 20px;
            }
            
            .info-section h3 {
                color: #1e3c72;
                font-size: 14px;
                text-transform: uppercase;
                margin-bottom: 10px;
                border-bottom: 2px solid #2a5298;
                padding-bottom: 5px;
            }
            
            .info-value {
                display: flex;
                justify-content: space-between;
                padding: 8px 0;
                border-bottom: 1px solid #eee;
                font-size: 13px;
            }
            
            .info-value:last-child {
                border-bottom: none;
            }
            
            .label {
                font-weight: 600;
                color: #555;
            }
            
            .value {
                color: #2a5298;
                font-weight: 500;
            }
            
            .controls {
                background: rgba(255, 255, 255, 0.95);
                border-radius: 10px;
                padding: 20px;
                box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            }
            
            button {
                width: 100%;
                padding: 12px;
                background: #2a5298;
                color: white;
                border: none;
                border-radius: 5px;
                cursor: pointer;
                font-weight: 600;
                transition: background 0.3s;
                margin-bottom: 10px;
            }
            
            button:hover {
                background: #1e3c72;
            }
            
            button.secondary {
                background: #95a5a6;
            }
            
            button.secondary:hover {
                background: #7f8c8d;
            }
            
            .error-message {
                color: #e74c3c;
                font-size: 12px;
                padding: 10px;
                background: #fadbd8;
                border-radius: 5px;
                margin-bottom: 10px;
                display: none;
            }
            
            .error-message.show {
                display: block;
            }
            
            footer {
                text-align: center;
                color: rgba(255, 255, 255, 0.7);
                padding-top: 20px;
                font-size: 12px;
            }
        </style>
    </head>
    <body>
        <div class="container">
            <header>
                <h1>🛰️ FMCW Radar - FFT Frequency Domain Visualization</h1>
                <div class="status-bar">
                    <div class="status-item">
                        <span class="status-indicator" id="statusIndicator"></span>
                        <span id="statusText">Disconnected</span>
                    </div>
                    <div class="status-item">
                        <span>Packets: <strong id="packetCount">0</strong></span>
                    </div>
                    <div class="status-item">
                        <span>Errors: <strong id="errorCount">0</strong></span>
                    </div>
                    <div class="status-item">
                        <span>Last Update: <strong id="lastUpdate">-</strong></span>
                    </div>
                </div>
            </header>
            
            <div class="main-content">
                <div class="chart-container">
                    <div id="plot"></div>
                </div>
                
                <div>
                    <div class="sidebar">
                        <div class="info-section">
                            <h3>Peak Information</h3>
                            <div class="info-value">
                                <span class="label">Frequency</span>
                                <span class="value" id="peakFreqHz">-</span>
                            </div>
                            <div class="info-value">
                                <span class="label">Frequency (MHz)</span>
                                <span class="value" id="peakFreqMHz">-</span>
                            </div>
                            <div class="info-value">
                                <span class="label">Magnitude</span>
                                <span class="value" id="peakMag">-</span>
                            </div>
                        </div>
                        
                        <div class="info-section">
                            <h3>FFT Configuration</h3>
                            <div class="info-value">
                                <span class="label">FFT Size</span>
                                <span class="value">1024</span>
                            </div>
                            <div class="info-value">
                                <span class="label">Sample Rate</span>
                                <span class="value">20 MSPS</span>
                            </div>
                            <div class="info-value">
                                <span class="label">Bins</span>
                                <span class="value">512</span>
                            </div>
                            <div class="info-value">
                                <span class="label">Resolution</span>
                                <span class="value">19.5 kHz</span>
                            </div>
                        </div>
                        
                        <div class="info-section">
                            <h3>Serial Connection</h3>
                            <div class="info-value">
                                <span class="label">Port</span>
                                <span class="value" id="serialPort">-</span>
                            </div>
                            <div class="info-value">
                                <span class="label">Baud Rate</span>
                                <span class="value" id="serialBaud">-</span>
                            </div>
                        </div>
                    </div>
                    
                    <div class="controls">
                        <div class="error-message" id="errorMessage"></div>
                        <button onclick="resetPlot()">Reset Plot</button>
                        <button class="secondary" onclick="exportData()">Export Data</button>
                    </div>
                </div>
            </div>
            
            <footer>
                <p>FMCW Radar FFT Visualization | Real-time frequency domain analysis</p>
            </footer>
        </div>
        
        <script>
            let updateInterval;
            let lastPacketCount = 0;
            
            function updatePlot() {
                fetch('/api/fft_data')
                    .then(response => response.json())
                    .then(data => {
                        // Convert frequencies to MHz for display
                        const freqMHz = data.frequencies.map(f => f / 1e6);
                        
                        const trace = {
                            x: freqMHz,
                            y: data.magnitude,
                            type: 'scatter',
                            mode: 'lines',
                            name: 'Magnitude',
                            line: {
                                color: '#2a5298',
                                width: 2
                            },
                            fill: 'tozeroy',
                            fillcolor: 'rgba(42, 82, 152, 0.1)'
                        };
                        
                        const layout = {
                            title: {
                                text: 'FFT Magnitude Spectrum',
                                font: { size: 18, color: '#1e3c72' }
                            },
                            xaxis: {
                                title: 'Frequency (MHz)',
                                gridcolor: '#eee'
                            },
                            yaxis: {
                                title: 'Magnitude',
                                gridcolor: '#eee'
                            },
                            plot_bgcolor: 'rgba(250, 250, 250, 0.5)',
                            paper_bgcolor: 'rgba(255, 255, 255, 0)',
                            margin: { l: 60, r: 20, t: 50, b: 50 },
                            hovermode: 'closest'
                        };
                        
                        Plotly.newPlot('plot', [trace], layout, { responsive: true });
                        
                        // Update peak information
                        document.getElementById('peakFreqHz').textContent = 
                            data.peak_frequency ? data.peak_frequency.toFixed(0) + ' Hz' : '-';
                        document.getElementById('peakFreqMHz').textContent = 
                            data.peak_frequency ? (data.peak_frequency / 1e6).toFixed(3) + ' MHz' : '-';
                        document.getElementById('peakMag').textContent = 
                            data.peak_magnitude ? data.peak_magnitude.toFixed(4) : '-';
                    })
                    .catch(error => {
                        console.error('Error fetching FFT data:', error);
                        showError('Failed to fetch FFT data');
                    });
            }
            
            function updateStatus() {
                fetch('/api/stats')
                    .then(response => response.json())
                    .then(data => {
                        const indicator = document.getElementById('statusIndicator');
                        const statusText = document.getElementById('statusText');
                        
                        if (data.is_connected) {
                            indicator.classList.add('connected');
                            statusText.textContent = 'Connected';
                        } else {
                            indicator.classList.remove('connected');
                            statusText.textContent = 'Disconnected';
                        }
                        
                        document.getElementById('packetCount').textContent = data.packets_received;
                        document.getElementById('errorCount').textContent = data.packets_errors;
                        
                        if (data.packets_received > lastPacketCount) {
                            lastPacketCount = data.packets_received;
                            document.getElementById('lastUpdate').textContent = new Date().toLocaleTimeString();
                        }
                    })
                    .catch(error => console.error('Error fetching stats:', error));
            }
            
            function updateConnectionInfo() {
                fetch('/api/connection_status')
                    .then(response => response.json())
                    .then(data => {
                        document.getElementById('serialPort').textContent = data.port;
                        document.getElementById('serialBaud').textContent = data.baud + ' baud';
                    })
                    .catch(error => console.error('Error fetching connection info:', error));
            }
            
            function resetPlot() {
                Plotly.purge('plot');
                document.getElementById('peakFreqHz').textContent = '-';
                document.getElementById('peakFreqMHz').textContent = '-';
                document.getElementById('peakMag').textContent = '-';
            }
            
            function exportData() {
                fetch('/api/fft_data')
                    .then(response => response.json())
                    .then(data => {
                        const csv = 'Frequency (Hz),Frequency (MHz),Magnitude,Phase\\n';
                        let csvContent = csv;
                        
                        for (let i = 0; i < data.magnitude.length; i++) {
                            csvContent += data.frequencies[i] + ',' + 
                                        (data.frequencies[i] / 1e6).toFixed(6) + ',' +
                                        data.magnitude[i].toFixed(6) + ',' +
                                        data.phase[i].toFixed(6) + '\\n';
                        }
                        
                        const element = document.createElement('a');
                        element.setAttribute('href', 'data:text/csv;charset=utf-8,' + encodeURIComponent(csvContent));
                        element.setAttribute('download', 'fft_data_' + new Date().getTime() + '.csv');
                        element.style.display = 'none';
                        document.body.appendChild(element);
                        element.click();
                        document.body.removeChild(element);
                    })
                    .catch(error => {
                        console.error('Error exporting data:', error);
                        showError('Failed to export data');
                    });
            }
            
            function showError(message) {
                const errorDiv = document.getElementById('errorMessage');
                errorDiv.textContent = message;
                errorDiv.classList.add('show');
                setTimeout(() => errorDiv.classList.remove('show'), 5000);
            }
            
            // Initialize
            window.addEventListener('load', function() {
                updateConnectionInfo();
                updatePlot();
                updateStatus();
                
                // Update every 500ms
                updateInterval = setInterval(function() {
                    updatePlot();
                    updateStatus();
                }, 500);
            });
            
            window.addEventListener('beforeunload', function() {
                if (updateInterval) clearInterval(updateInterval);
            });
        </script>
    </body>
    </html>
    """
    return render_template_string(html)


@app.route('/api/fft_data')
def get_fft_data():
    """Get current FFT data as JSON."""
    with data_lock:
        return jsonify({
            'magnitude': fft_data['magnitude'],
            'frequencies': fft_data['frequencies'],
            'phase': fft_data['phase'],
            'sample_rate': fft_data['sample_rate'],
            'timestamp': fft_data['timestamp'],
            'peak_frequency': fft_data['peak_frequency'],
            'peak_magnitude': fft_data['peak_magnitude']
        })


@app.route('/api/stats')
def get_stats():
    """Get connection and packet statistics."""
    with data_lock:
        return jsonify(stats)


@app.route('/api/connection_status')
def connection_status():
    """Get serial connection status."""
    return jsonify({
        'connected': stats['is_connected'],
        'port': SERIAL_PORT,
        'baud': SERIAL_BAUD,
        'packets_received': stats['packets_received'],
        'packets_errors': stats['packets_errors']
    })


@app.route('/api/peak_info')
def get_peak_info():
    """Get peak frequency and magnitude information."""
    with data_lock:
        return jsonify({
            'peak_frequency_hz': fft_data['peak_frequency'],
            'peak_frequency_mhz': fft_data['peak_frequency'] / 1e6,
            'peak_magnitude': fft_data['peak_magnitude'],
            'timestamp': fft_data['timestamp']
        })


@app.errorhandler(404)
def not_found(error):
    """Handle 404 errors."""
    return jsonify({'error': 'Not found'}), 404


@app.errorhandler(500)
def server_error(error):
    """Handle 500 errors."""
    logger.error(f"Server error: {error}")
    return jsonify({'error': 'Internal server error'}), 500


if __name__ == '__main__':
    try:
        # Start serial reader
        start_serial_reader()
        
        # Start Flask server
        logger.info("Starting Flask server on http://localhost:5000")
        print("\n" + "="*60)
        print("FMCW Radar FFT Visualization Server")
        print("="*60)
        print(f"Web Interface: http://localhost:5000")
        print(f"Serial Port: {SERIAL_PORT} @ {SERIAL_BAUD} baud")
        print(f"FFT Size: {FFT_SIZE} points")
        print(f"Sample Rate: {SAMPLE_RATE_HZ / 1e6} MSPS")
        print("="*60 + "\n")
        
        app.run(host='127.0.0.1', port=5000, debug=False, use_reloader=False)
    except KeyboardInterrupt:
        logger.info("Keyboard interrupt received")
    finally:
        # Cleanup
        stop_serial_reader()
        logger.info("Application stopped")
