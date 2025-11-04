# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an ESP32/ESP8266 project template built with PlatformIO and the Arduino framework. It provides a production-ready foundation for IoT devices with WiFi connectivity, web-based configuration, OTA updates, and EEPROM persistence.

## Build System (PlatformIO)

**Build for ESP32:**
```bash
pio run -e esp32dev
```

**Build for ESP8266:**
```bash
pio run -e nodemcu
```

**Upload firmware:**
```bash
pio run -e esp32dev -t upload    # ESP32
pio run -e nodemcu -t upload     # ESP8266
```

**Monitor serial output:**
```bash
pio device monitor
```

**Clean build:**
```bash
pio run -t clean
```

**Important:** The `extra_script_pre.py` automatically increments the patch version in `src/common/version.h` before each build.

## Code Architecture

### Dual-Platform Structure

The codebase uses a split architecture to support both ESP32 and ESP8266:

- **`src/common/`** - Shared platform-agnostic code used by both ESP32 and ESP8266
  - Core handlers: WiFi, server, OTA, EEPROM
  - Configuration management
  - Utilities and common functionality

- **`src/`** - Platform-specific implementation
  - `main.cpp` - Entry point (calls `commonSetup()` and `commonLoop()`)
  - `globals.h/cpp` - Project-specific global variables
  - `serverHandles.h/cpp` - Project-specific server routes
  - `system_config.h/cpp` - Project-specific EEPROM configuration structs

### Core Subsystems

**WiFi Management** (`src/common/wifi_handler.cpp`)
- Handles two modes: Configuration Mode and Run Mode
- Configuration mode activates when:
  - No valid EEPROM configuration exists
  - Device restarts within 30 seconds (quick restart detection)
- Exposes `ArduinoNetConfig` AP in configuration mode
- Configuration accessible at `http://arduino.local/configure`

**Web Server** (`src/common/server_handler.cpp`)
- Built on ESPAsyncWebServer
- Default routes:
  - `/` - Home page
  - `/reboot` - Reboot device
  - `/configure` - Configuration page
  - `/invalidateConfig` - Force configuration mode on next restart
  - `/checkForUpdates` - Manual OTA update check
  - `/uploadFirmware` - Browser-based firmware upload

**OTA Updates** (`src/common/ota_handler.cpp`)
- GitHub release-based automatic updates
- Supports private repositories with authentication tokens
- Class: `ESPGithubOtaUpdate`
- Checks GitHub releases for newer versions
- Downloads and flashes firmware automatically

**EEPROM Configuration** (`src/common/device_configuration.h`, `src/common/eeprom_utils.tpp`)
- Template-based generic EEPROM save/load system
- Uses `#pragma pack(push, 1)` to prevent struct padding issues
- Two configuration types:
  - `DeviceConfiguration` - WiFi credentials, hostname, GitHub token (common)
  - `SystemConfiguration` - Project-specific settings (in `src/system_config.h`)
- Quick restart detection prevents boot loops

**Watchdog**
- Default 30-second timeout
- Prevents device hangs

### Version Management

Version is stored in `src/common/version.h`:
```cpp
const char *SW_VERSION = "1.2.6";
```

The pre-build script (`extra_script_pre.py`) auto-increments the patch version before each build.

## Configuration Pattern

### Adding New EEPROM Configuration

1. Define struct in `src/system_config.h` with `#pragma pack(push, 1)`:
```cpp
struct SystemConfiguration {
    char myConfig;
    // Add fields here
};
```

2. Implement save/load in `src/system_config.cpp`:
```cpp
bool readConfigFromEeprom() {
    return readFromEeprom<SystemConfiguration>(SYSTEM_EEPROM_ADDRESS, systemConfiguration);
}

void saveConfigToEeprom() {
    saveToEeprom<SystemConfiguration>(SYSTEM_EEPROM_ADDRESS, systemConfiguration);
}
```

3. Call in `main.cpp`:
```cpp
void setup() {
    commonSetup();
    readConfigFromEeprom();
    // ...
}
```

### Adding Server Routes

1. Declare in `src/serverHandles.h`:
```cpp
void myCustomRoute(AsyncWebServerRequest *request);
```

2. Implement in `src/serverHandles.cpp`:
```cpp
void myCustomRoute(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Response");
}
```

3. Register in `addServerHandles()` function:
```cpp
void addServerHandles() {
    server.on("/myroute", HTTP_GET, myCustomRoute);
}
```

## Key Files

- `platformio.ini` - Build configuration for both ESP32 and ESP8266
- `src/main.cpp` - Entry point (minimal, calls common functions)
- `src/common/common_main.cpp` - Core setup and loop logic
- `src/common/device_configuration.h` - WiFi and device config structs
- `src/common/version.h` - Software version (auto-incremented)
- `src/globals.h` - Project-specific global variables
- `extra_script_pre.py` - Auto-increments version before build
- `extra_script_post.py` - Opens serial monitor after upload

## Dependencies

Managed via PlatformIO `lib_deps`:
- **AsyncTCP** (ESP32) or **ESPAsyncTCP** (ESP8266) - Async networking
- **ESPAsyncWebServer** - Async web server
- **ArduinoJson** (v7) - JSON parsing/serialization
- **OneWire** (ESP8266 only) - OneWire protocol support

## Development Workflow

1. Modify `src/common/version.h` to set base version
2. Implement project-specific logic in `src/main.cpp`
3. Add custom configuration structs in `src/system_config.h`
4. Add server routes in `src/serverHandles.cpp`
5. Build with `pio run -e esp32dev` or `pio run -e nodemcu`
6. Upload with `pio run -e <env> -t upload`
7. Version auto-increments on each build

## Testing

This is an embedded project without a traditional test framework. Testing is done:
- Via serial monitor output (`pio device monitor`)
- Through web interface routes (`http://<hostname>/`)
- By observing device behavior after OTA updates

## Important Notes

- **Never commit secrets**: GitHub tokens, WiFi passwords should be in EEPROM configuration, not hardcoded
- **EEPROM addresses**: Ensure no overlap between `DeviceConfiguration` and `SystemConfiguration` addresses
- **Struct packing**: Always use `#pragma pack(push, 1)` for EEPROM structs to prevent alignment issues
- **Platform differences**: Check platform-specific code paths with `#ifdef ESP32` or `#ifdef ESP8266`
- **OTA updates**: Require GitHub releases with binary assets matching the configured binary name
