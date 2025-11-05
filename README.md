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

### Syncing Template Updates with Git Subtree

This template uses **git subtree** to enable automatic syncing of the `src/common/` folder between your project and the template. This means you can:
- Pull template improvements into your project with one command
- Push your improvements back to the template to share with other projects

#### Initial Setup (One-Time)

**When starting a new project from this template:**

```bash
# 1. Copy or clone the template
cp -r /path/to/esp-project-template my-new-project
cd my-new-project

# 2. Initialize git (if not already done)
git init
git add .
git commit -m "Initial commit from ESP template"

# 3. Add template as a remote
git remote add template /path/to/esp-project-template

# 4. Verify remote was added
git remote -v
# Should show:
# template  /path/to/esp-project-template (fetch)
# template  /path/to/esp-project-template (push)
```

**Note:** If you copied the template (not cloned), the `src/common/` folder already exists, so you're ready to sync immediately. No need to run `git subtree add`.

#### Pulling Template Updates

**When to pull:** Pull updates from the template when you want the latest bug fixes and improvements.

```bash
cd /path/to/your-project
git subtree pull --prefix=src/common template main --squash
```

**What this does:**
- Fetches latest changes from the template repository
- Merges them into your `src/common/` folder
- Creates a single squashed commit
- **Does NOT affect** your project-specific files (main.cpp, dht_handler.cpp, etc.)

**Example output:**
```
Subtree is already at commit abc123
```
or
```
Merge made by the 'recursive' strategy.
 src/common/wifi_handler.cpp | 3 +++
 1 file changed, 3 insertions(+)
```

#### Pushing Improvements Back

**When to push:** When you fix a bug or add a feature to `src/common/` that should benefit other projects.

```bash
cd /path/to/your-project

# 1. Make your changes in src/common/
# 2. Commit normally
git add src/common/wifi_handler.cpp
git commit -m "Fix WiFi reconnection timeout"

# 3. Push to template (one command!)
git subtree push --prefix=src/common template main
```

**What this does:**
- Extracts commits that touched `src/common/`
- Pushes them to the template repository
- Maintains git history
- Other projects can then pull your improvements

#### What Gets Synced vs. What Doesn't

**✅ Synced via subtree (src/common/):**
- `common_main.cpp/h` - Core setup and loop
- `wifi_handler.cpp/h` - WiFi management
- `server_handler.cpp/h` - Web server
- `server_handles.cpp` - Built-in HTTP routes
- `ota_handler.cpp/h` - OTA updates
- `device_configuration.cpp/h` - Configuration structs
- `eeprom_utils.tpp` - EEPROM utilities
- `memory_stats.cpp/h` - RAM monitoring
- `power_monitor.h` - VCC monitoring
- `esp8266_ota_update.cpp/h` - ESP8266 OTA
- `utils.cpp/h` - Utilities
- `globals.h` - Common globals
- `common_config.cpp` - Configuration constants
- `version.h` - Software version

**❌ Never synced (project-specific):**
- `src/main.cpp` - Your project entry point
- `src/globals.cpp/h` - Project-specific globals
- `src/serverHandles.cpp/h` - Project-specific routes
- `src/dht_handler.cpp` - Sensor handlers (example)
- `src/system_config.cpp/h` - Project EEPROM config
- `platformio.ini` - May have project dependencies
- `README.md`, `.gitignore`, etc.

#### Handling Merge Conflicts

If you modified `src/common/` in your project AND the template was also updated, you may get conflicts:

```bash
# After git subtree pull
# If conflicts occur:
git status  # See conflicting files

# Edit conflicting files manually
# Look for <<<<<<< HEAD markers
# Choose which changes to keep

# After resolving:
git add <resolved-files>
git commit -m "Merge template updates, resolved conflicts"
```

#### Best Practices

**Do pull regularly:**
```bash
# Before starting new features
git subtree pull --prefix=src/common template main --squash
```

**Do push improvements:**
```bash
# After fixing bugs in common/ code
git subtree push --prefix=src/common template main
```

**Don't modify common/ unnecessarily:**
- Keep project-specific code in `src/` (not `src/common/`)
- Only modify `src/common/` for true improvements

**Do commit before pulling:**
```bash
# Always commit your work first
git add .
git commit -m "Work in progress"
# Then pull
git subtree pull --prefix=src/common template main --squash
```

#### Troubleshooting

**"fatal: refusing to merge unrelated histories"**
```bash
# Add --allow-unrelated-histories flag
git subtree pull --prefix=src/common template main --squash --allow-unrelated-histories
```

**"fatal: prefix 'src/common' already exists"**
- This means `src/common/` already exists (good!)
- Skip `git subtree add`, go straight to `git subtree pull`

**"Working tree has modifications"**
```bash
# Commit your changes first
git add .
git commit -m "Save work before sync"
# Then try again
git subtree pull --prefix=src/common template main --squash
```

#### Quick Reference Card

```bash
# Setup (one-time)
git remote add template /path/to/esp-project-template
git remote -v  # Verify

# Pull updates (regular)
git subtree pull --prefix=src/common template main --squash

# Push improvements (when you fix/improve common/ code)
git subtree push --prefix=src/common template main

# Check if template remote exists
git remote -v | grep template
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
