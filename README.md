# LED Controller for ESP32-C3

This project is a firmware for the ESP32-C3 board, providing control over a PIR sensor and an LED. It supports WebSocket communication and OTA updates.

## Features
- **Two operation modes:**
  - Manual LED control via Web interface.
  - Automatic LED activation based on PIR sensor detection.
- **Smooth LED transitions** using PWM.
- **Wi-Fi configuration:**
  - AP mode for initial setup.
  - Connecting to selected SSID.
- **Time synchronization** upon Wi-Fi connection (in the process of debugging).
- **mDNS support** (default led-controller.local).
- **Error indication** using the built-in LED of the board.
- **OTA updates** for firmware upgrades.
- **Logging:**
  - The last few log entries are available via the web interface.
  - Logs are also output to UART/Serial.

## Configuration
- **Hardware settings** are located in `config.h`.
- **Default values** are set in `config_manager.cpp`.

## Installation
### 1. Clone the repository:
```sh
git clone https://github.com/pa-pe/led-controller-esp32.git
cd led-controller-esp32
```

### 2. Install dependencies:
Ensure you have **Arduino IDE** or **PlatformIO** installed with the ESP32 board package.

### 3. Flash the firmware:
For **Arduino IDE**:
- Select **ESP32-C3** as the board.
- Set the correct **port**.
- Upload the code.

For **PlatformIO**:
```sh
pio run --target upload
```

### 4. Connect to the Web Interface:
1. If the device is in AP mode, connect to `led-controller-esp` and go to `http://192.168.4.1`
2. If connected to Wi-Fi, access `http://led-controller.local`

## License
This project is licensed under the MIT License.
