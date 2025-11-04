# ESP Project Template

**Unified template for ESP32 and ESP8266 IoT projects**

This is a production-ready template that supports both ESP32 and ESP8266 boards with a shared codebase. Ideal for quickly starting new IoT projects with professional features already implemented.

## ✨ Features

### 🔌 Dual Platform Support
- **ESP32** (ESP32-DEV, NodeMCU-32S, and compatible boards)
- **ESP8266** (NodeMCU v2, v3, ESP-01, and compatible boards)
- 95% code sharing between platforms
- Platform-specific code isolated with compiler guards

### 📡 WiFi Management
- Automatic configuration mode on first boot
- Configuration web interface at `http://arduino.local/configure`
- Quick restart detection for entering config mode
- Boot loop prevention (safety feature)
- WiFi sleep disabled for stability
- mDNS support for easy device discovery

### 🔄 OTA Updates
- GitHub release-based automatic updates
- Private repository support with authentication tokens
- Browser-based firmware upload
- Safe update mechanism with rollback protection

### 🌐 Web Server
- Async web server (non-blocking)
- Built-in routes:
  - `/` - Home page with system info
  - `/configureDevice` - Configuration interface
  - `/reboot` - Device restart
  - `/invalidateConfig` - Force reconfiguration
  - `/checkForUpdates` - Manual OTA check
  - `/logsStream` - WebSocket real-time logs
  - `/uploadFirmware` - Browser firmware upload
- Extensible routing system

### 💾 Configuration Management
- EEPROM-based persistent configuration
- Checksum validation
- Packed structs (no padding issues)
- Configurable settings:
  - WiFi credentials
  - Device hostname
  - Device name
  - GitHub auth token
  - LED alive signal enable/disable

### 📊 Monitoring
- Memory usage stats (ESP32/ESP8266 RAM tracking)
- Power monitoring with VCC logging (ESP8266)
- Reset cause detection
- Boot loop detection and prevention
- WebSocket-based log streaming
- Serial and web console output

### 💡 LED Alive Signal
- Configurable heartbeat LED flash
- Can be enabled/disabled via web configuration
- Indicates device is alive and running
- Inverted logic support for different boards

### 🔧 Developer Features
- Auto-incrementing version numbers
- Serial monitor auto-opens after upload
- Dual logging (Serial + WebSocket)
- Debug macros for conditional logging
- Template-based EEPROM utilities
- Clean project structure

## 📋 Requirements

### Hardware
- **ESP32 board** (ESP32-DEV, NodeMCU-32S, etc.) OR
- **ESP8266 board** (NodeMCU v2/v3, ESP-01, etc.)
- USB cable for programming
- Optional: External LED for projects

### Software
- [PlatformIO](https://platformio.org/) (VS Code extension or CLI)
- Git (for OTA updates from GitHub)

## 🚀 Quick Start

### 1. Clone or Download Template
```bash
# Clone the template
git clone <repository-url> my-new-project
cd my-new-project

# Or use GitHub's "Use this template" button
```

### 2. Choose Your Platform

**For ESP32:**
```bash
pio run -e esp32dev
```

**For ESP8266:**
```bash
pio run -e nodemcu
```

### 3. Upload Firmware

```bash
# ESP32
pio run -e esp32dev -t upload

# ESP8266
pio run -e nodemcu -t upload
```

Serial monitor will automatically open after upload.

### 4. Configure Device

1. Device starts in configuration mode (LED flashing)
2. Connect to WiFi AP: **ArduinoNet** (no password)
3. Navigate to `http://arduino.local/configure`
4. Enter your WiFi credentials and device settings
5. Click **Save**
6. Device connects to your WiFi and reboots

## 📁 Project Structure

```
esp-project-template/
├── platformio.ini              # Build configuration (ESP32 + ESP8266)
├── extra_script_pre.py         # Auto-increment version before build
├── extra_script_post.py        # Open serial monitor after upload
├── src/
│   ├── main.cpp                # Your project entry point
│   ├── globals.h/cpp           # Project-specific globals
│   ├── serverHandles.h/cpp     # Project-specific HTTP routes
│   └── common/                 # Shared platform-agnostic code ⭐
│       ├── common_main.h/cpp   # Core setup and loop
│       ├── device_configuration.h/cpp  # Configuration structs
│       ├── wifi_handler.h/cpp  # WiFi management
│       ├── server_handler.h/cpp # Web server
│       ├── server_handles.cpp  # Built-in HTTP routes
│       ├── ota_handler.h/cpp   # OTA updates
│       ├── esp8266_ota_update.h/cpp # ESP8266 OTA implementation
│       ├── eeprom_utils.tpp    # EEPROM utilities
│       ├── memory_stats.h/cpp  # RAM monitoring
│       ├── power_monitor.h     # VCC/reset monitoring
│       ├── common_config.cpp   # Configuration constants
│       ├── globals.h           # Common globals
│       ├── utils.h/cpp         # Utility functions
│       └── version.h           # Software version (auto-updated)
└── README.md                   # This file
```

## 🛠️ Customization

### Adding Project-Specific Code

**Example: Add a temperature sensor**

1. **Create handlers** (`src/dht_handler.h/cpp`)
2. **Add routes** in `src/serverHandles.cpp`:
   ```cpp
   void addServerHandles() {
       webServer->on("/temperature", HTTP_GET, routeGetTemperature);
       routeDescriptions["/temperature"] = "Get current temperature";
   }
   ```
3. **Update main.cpp**:
   ```cpp
   void setup() {
       commonSetup();
       setupDHT();  // Your sensor init
       addServerHandles();
   }

   void loop() {
       if (commonLoop() > 0) return;  // Handle config/boot modes
       dhtLoop();  // Your sensor reading
   }
   ```

### Adding EEPROM Configuration

**Example: Store sensor calibration**

1. **Define struct** in `src/system_config.h`:
   ```cpp
   #pragma pack(push, 1)
   struct SystemConfiguration {
       float temperatureOffset;
       int sampleInterval;
   };
   #pragma pack(pop)
   ```

2. **Calculate address** (must not overlap with DeviceConfiguration):
   ```cpp
   #define SYSTEM_CONFIG_ADDR nextEepromSlot<DeviceConfiguration>(DEVICE_CONFIGURATION_EEPROM_ADDR)
   ```

3. **Save/Load** in `src/system_config.cpp`:
   ```cpp
   bool readConfigFromEeprom() {
       return readFromEeprom<SystemConfiguration>(SYSTEM_CONFIG_ADDR, systemConfig);
   }

   void saveConfigToEeprom() {
       saveToEeprom<SystemConfiguration>(SYSTEM_CONFIG_ADDR, systemConfig);
   }
   ```

## 🔧 Build Commands

### ESP32
```bash
# Build
pio run -e esp32dev

# Upload
pio run -e esp32dev -t upload

# Monitor
pio device monitor

# Clean
pio run -e esp32dev -t clean
```

### ESP8266
```bash
# Build
pio run -e nodemcu

# Upload
pio run -e nodemcu -t upload

# Monitor
pio device monitor

# Clean
pio run -e nodemcu -t clean
```

### Both Platforms
```bash
# Build both
pio run

# Test compile for both platforms
pio run -e esp32dev && pio run -e nodemcu
```

## 📝 Configuration Settings

Access at `http://<device-ip>/configureDevice`:

| Setting | Description | Required |
|---------|-------------|----------|
| **WiFi SSID** | Your WiFi network name | ✅ Yes |
| **WiFi Password** | Your WiFi password | ✅ Yes |
| **Hostname** | Device hostname for mDNS | Optional |
| **Device Name** | Friendly name for device | Optional |
| **GitHub Token** | For private repo OTA updates | Optional |
| **LED Alive Signal** | Enable/disable heartbeat flash | Optional |

## 🔄 OTA Updates

### Setup GitHub Releases

1. Create a GitHub repository for your project
2. Tag a release: `git tag v1.0.0 && git push --tags`
3. Upload compiled firmware as release asset
4. Device automatically checks for updates hourly

### Manual Update Check
Navigate to: `http://<device-ip>/checkForUpdates`

### Browser Upload
Navigate to: `http://<device-ip>/uploadFirmware`

## 🐛 Debugging

### Serial Monitor
```bash
pio device monitor
```

### WebSocket Logs
Navigate to: `http://<device-ip>/logsStream`

Real-time log streaming in your browser!

### Debug Macros
```cpp
#define DEBUG  // Uncomment in src/common/globals.h

DEBUG_PRINTLN("Debug message");  // Only prints if DEBUG defined
LOG_PRINTLN("Always printed");   // Always prints (serial + WebSocket)
```

## 🔐 Security Notes

- **Never commit** WiFi passwords or GitHub tokens to git
- All credentials stored in EEPROM (device only)
- OTA updates use HTTPS
- Configuration page uses simple HTTP (consider VPN for remote access)

## 📚 Dependencies

Managed automatically by PlatformIO:
- **AsyncTCP** (ESP32) / **ESPAsyncTCP** (ESP8266) - Async networking
- **ESPAsyncWebServer** - Non-blocking web server
- **ArduinoJson** v7 - JSON parsing/serialization
- **OneWire** (ESP8266 only) - OneWire protocol support

## 🤝 Contributing

This template is designed to be forked and customized for your projects.

### Syncing Template Updates

If you want to pull template improvements into your project:

1. **Setup git subtree** (one-time):
   ```bash
   cd /path/to/your-project
   git remote add template /path/to/esp-project-template
   git subtree add --prefix=src/common template main --squash
   ```

2. **Pull updates** (anytime):
   ```bash
   git subtree pull --prefix=src/common template main --squash
   ```

3. **Push improvements back** (if you improve common/ code):
   ```bash
   git subtree push --prefix=src/common template main
   ```

## 📊 Version Management

Version is automatically incremented before each build:
- Located in `src/common/version.h`
- Format: `MAJOR.MINOR.PATCH` (e.g., "1.2.6")
- Patch version auto-increments
- Manual edit for major/minor changes

## 🏗️ Architecture Decisions

### Why Dual-Platform?
- Maximize code reuse (95% shared)
- Support both ESP32 (newer) and ESP8266 (legacy)
- Learn once, deploy anywhere

### Why `src/common/` folder?
- Clear separation between template and project code
- Easy to sync template updates
- Reduces merge conflicts

### Why Async Web Server?
- Non-blocking HTTP handling
- Better performance
- Supports WebSocket for real-time logs

### Why EEPROM instead of SPIFFS/LittleFS?
- Simpler for configuration data
- Faster read/write
- Less wear on flash
- Both platforms support it natively

## 📄 License

MIT License - Feel free to use in commercial and personal projects.

## 🙏 Credits

Built with [PlatformIO](https://platformio.org/) and [Arduino Framework](https://www.arduino.cc/).

Template created: November 2025
Last updated: November 2025

---

**Happy Building! 🚀**

If you build something cool with this template, let us know!
