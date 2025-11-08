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
- Exposes `ArduinoNet` AP in configuration mode (no password)
- Configuration accessible at `http://arduino.local/configureDevice`
- WiFi sleep is disabled for stability (`WiFi.setSleep(false)`)
- Connection health checks via periodic Google.com connectivity test

**Web Server** (`src/common/server_handler.cpp`, `src/common/server_handles.cpp`)
- Built on ESPAsyncWebServer
- Built-in common routes (in `src/common/server_handles.cpp`):
  - `/configureDevice` - Device WiFi and settings configuration
  - `/saveConfiguration` - Save device configuration (tests WiFi before saving)
  - `/reboot` - Reboot device
  - `/invalidateConfig` - Force configuration mode on next restart
  - `/checkForUpdates` - Manual OTA update check
  - `/uploadFirmware` - Browser-based firmware upload
  - `/logsStream` - WebSocket-based real-time log viewer
- Project routes (in `src/serverHandles.cpp`):
  - `/` - Home page (shows version, WiFi strength, quick restarts count, available routes)
  - `/configure` - Project-specific configuration page
- Route descriptions stored in `routeDescriptions` map for auto-documentation

**OTA Updates** (`src/common/ota_handler.cpp`)
- GitHub release-based automatic updates
- Supports private repositories with authentication tokens
- Class: `ESPGithubOtaUpdate`
- Checks GitHub releases for newer versions
- Downloads and flashes firmware automatically
- Firmware upload routes registered automatically

**EEPROM Configuration** (`src/common/device_configuration.h`, `src/common/eeprom_utils.tpp`)
- Template-based generic EEPROM save/load system
- Uses `#pragma pack(push, 1)` to prevent struct padding issues
- Two configuration types:
  - `DeviceConfiguration` - WiFi credentials, hostname, device name, GitHub token, LED alive signal (common)
  - `SystemConfiguration` - Project-specific settings (in `src/system_config.h`)
- Quick restart detection prevents boot loops
- Address calculation via `nextEepromSlot<T>()` template function
- EEPROM addresses initialized in `commonSetup()`:
  ```cpp
  JUST_RESTARTED_EEPROM_ADDR = 0;
  DEVICE_CONFIGURATION_EEPROM_ADDR = nextEepromSlot<QuickRestarts>(JUST_RESTARTED_EEPROM_ADDR);
  ```

**Watchdog**
- ESP32: 30-second timeout via `esp_task_wdt` (configurable via `watchdogTimeout_s`)
- ESP8266: 8-second timeout via `ESP.wdtEnable(WDTO_8S)`
- Fed regularly in `commonLoop()`
- Prevents device hangs

**Logging System**
- Dual logging to Serial and WebSocket clients
- Macros:
  - `LOG_PRINT(str)` / `LOG_PRINTLN(str)` - Always logs to both Serial and WebSocket
  - `DEBUG_PRINT(str)` / `DEBUG_PRINTLN(str)` - Only logs when `DEBUG` is defined
- Debug mode controlled by `#define DEBUG` in `src/common/globals.h`
- WebSocket endpoint: `/wsLogs` with HTML viewer at `/logsStream`

### Version Management

Version is stored in `src/common/version.h`:
```cpp
const char *SW_VERSION = "1.2.6";
```

The pre-build script (`extra_script_pre.py`) auto-increments the patch version before each build.

### Main Loop Architecture

The `commonLoop()` function returns status codes to control project execution:
- `2` - Boot loop mode (device restarted too many times, runs minimal functionality)
- `1` - Configuration mode (waiting for WiFi config)
- `0` - Normal operation (fully configured and running)

Typical `main.cpp` loop pattern:
```cpp
void loop() {
    if (commonLoop() == 2) {  // Boot loop protection
        delay(500);
        return;
    }

    if (!commonLoop() || systemConfiguration == nullptr) {  // Config mode or no config
        delay(500);
        return;
    }

    // Your project code here
}
```

## Configuration Pattern

### Adding New EEPROM Configuration

1. Calculate EEPROM address in `src/main.cpp` setup:
```cpp
void setup() {
    commonSetup();  // Initializes DEVICE_CONFIGURATION_EEPROM_ADDR

    // Calculate address for system config (must not overlap)
    SYSTEM_CONFIGURATION_EEPROM_ADDR = nextEepromSlot<DeviceConfiguration>(DEVICE_CONFIGURATION_EEPROM_ADDR);

    readConfigFromEeprom();
    // ...
}
```

2. Define struct in `src/system_config.h` with `#pragma pack(push, 1)`:
```cpp
#pragma pack(push, 1)
struct SystemConfiguration {
    char myConfig;
    // Add fields here

    SystemConfiguration() {}  // Default constructor

    String toStr() const {  // For debugging
        return "MyConfig: " + String(myConfig);
    }
};
#pragma pack(pop)
```

3. Implement save/load in `src/system_config.cpp`:
```cpp
SystemConfiguration *systemConfiguration = nullptr;

bool readConfigFromEeprom() {
    SystemConfiguration *eepromConfig = readDataFromEeprom<SystemConfiguration>(SYSTEM_CONFIGURATION_EEPROM_ADDR);
    if (eepromConfig != nullptr) {
        systemConfiguration = eepromConfig;
        return true;
    }
    return false;
}

void saveConfigToEeprom() {
    if (systemConfiguration == nullptr) return;
    writeDataToEeprom<SystemConfiguration>(SYSTEM_CONFIGURATION_EEPROM_ADDR, systemConfiguration);
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
    webServer->on("/myroute", HTTP_GET, myCustomRoute);
    routeDescriptions["/myroute"] = "Description for home page";
}
```

## Key Files

- `platformio.ini` - Build configuration for both ESP32 and ESP8266
- `src/main.cpp` - Entry point (minimal, calls common functions)
- `src/common/common_main.cpp` - Core setup and loop logic
- `src/common/device_configuration.h` - WiFi and device config structs
- `src/common/server_handles.cpp` - Built-in HTTP routes (configuration, OTA, logs)
- `src/serverHandles.cpp` - Project-specific HTTP routes
- `src/common/version.h` - Software version (auto-incremented)
- `src/common/globals.h` - Common globals and logging macros
- `src/globals.h` - Project-specific global variables
- `src/common/eeprom_utils.tpp` - Template-based EEPROM utilities
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
- WebSocket log viewer (`http://<hostname>/logsStream`)
- By observing device behavior after OTA updates

## Important Notes

- **Never commit secrets**: GitHub tokens, WiFi passwords should be in EEPROM configuration, not hardcoded
- **EEPROM addresses**: Use `nextEepromSlot<T>()` to calculate addresses and ensure no overlap
- **Struct packing**: Always use `#pragma pack(push, 1)` for EEPROM structs to prevent alignment issues
- **Platform differences**: Check platform-specific code paths with `#ifdef ESP32` or `#ifdef ESP8266`
- **OTA updates**: Require GitHub releases with binary assets matching the configured binary name
- **LED flash serial corruption**: On ESP32, LED operations can corrupt serial output. The code includes UART restoration logic after LED flashes.
- **Configuration testing**: When saving WiFi config, connection is tested before writing to EEPROM
- **Boot loop protection**: Device enters boot loop mode if restarted 5+ times within 30 seconds

## Security Considerations

### OTA Update Security

**CRITICAL**: The current OTA implementation uses `setInsecure()` which disables SSL certificate verification. This creates a **Man-in-the-Middle (MITM) vulnerability** where attackers can intercept OTA updates and inject malicious firmware.

**Current implementation** (in `src/common/ota_handler.cpp:40`):
```cpp
WiFiClientSecure secureClient;
secureClient.setInsecure(); // Skip certificate verification - INSECURE!
```

**Recommended improvements** (future enhancement):

For ESP32:
```cpp
// Add GitHub root certificate (ISRG Root X1 - Let's Encrypt)
const char* github_root_ca = "-----BEGIN CERTIFICATE-----\n"
    "MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw..."
    "-----END CERTIFICATE-----\n";
secureClient.setCACert(github_root_ca);
```

For ESP8266:
```cpp
// Option 1: Use certificate (higher memory usage)
secureClient.setCACert(github_root_ca);

// Option 2: Use fingerprint (lower memory, less secure)
secureClient.setFingerprint("GitHub cert fingerprint");

// Option 3: Use time-based validation with NTP
secureClient.setX509Time(time(nullptr));
```

**Additional security measures needed:**
- NTP time synchronization for proper SSL/TLS validation
- Firmware signature verification before flashing
- ESP32 secure boot features (if hardware supports it)
- Rollback protection to prevent downgrade attacks

### WiFi Security

- **WiFi persistence disabled**: `WiFi.persistent(false)` prevents credentials from being saved to flash, reducing flash wear and security risks
- **Auto-reconnect disabled**: `WiFi.setAutoReconnect(false)` gives full control over connection management
- **Sleep mode disabled**: Improves stability and reduces connection issues

## Recent Improvements (2025)

### WiFi Handler Enhancements
- **WiFi persistence control**: Prevents flash wear and unexpected reconnections
- **Improved mDNS reliability**:
  - ESP8266 gets 2-second delay (vs 1-second for ESP32) for cleanup
  - Automatic retry on mDNS initialization failure
  - HTTP service advertisement for better discoverability
- **Connection health monitoring**: Client connections properly closed after health checks
- **getIPAddress() utility**: Returns IP based on current WiFi mode (STA/AP)

### OTA Handler Enhancements
- **WiFi connection check**: Validates connection before attempting updates
- **Watchdog feeding**: Prevents timeout during long OTA downloads
  - Progress callbacks feed watchdog every update chunk
  - Critical for ESP8266 with 8-second watchdog timeout
- **HTTP client timeouts**: 30s total, 10s connect timeout
- **GitHub API compliance**: Proper headers (User-Agent, Accept)
- **Improved error handling**:
  - All messages use `LOG_PRINTLN` for WebSocket visibility
  - Better error messages for troubleshooting
  - File validation (.bin extension check)
  - Upload error state tracking prevents partial updates

### Memory & Resource Management
- **Memory statistics**: 24-hour tracking with min/max/average heap usage
- **Power monitoring**: VCC voltage tracking with brownout detection
- **Boot loop protection**: Sophisticated quick-restart counting system
- **Configuration safety**: WiFi credentials tested before EEPROM save

## Platform-Specific Best Practices

### ESP32
- **Watchdog**: 45-second timeout (configurable via `watchdogTimeout_s`)
- **LED Pin**: GPIO 2 (built-in LED on NodeMCU-32S)
- **WiFi sleep**: Explicitly disabled with `WiFi.setSleep(false)`
- **UART corruption prevention**: Serial flushed before LED operations, 1ms delay after restoration
- **Task WDT**: Uses ESP-IDF's `esp_task_wdt_reset()`

### ESP8266
- **Watchdog**: 8-second hardware timeout (non-configurable)
- **LED Pin**: GPIO 2 (avoid GPIO 1 - conflicts with TX)
- **WiFi sleep**: Disabled with `WiFi.setSleepMode(WIFI_NONE_SLEEP)`
- **mDNS**: Requires `MDNS.update()` in loop
- **OTA Upload**: Separate web server on port 8888 due to AsyncWebServer upload bugs
- **Memory constraints**: 80KB heap - be mindful of allocations
- **Deprecated APIs**: Use `(ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000` for proper size calculation

## Troubleshooting

### WiFi Issues
- **Can't connect to WiFi**: Check if credentials are correct via `/configureDevice`
- **Random disconnections**: Ensure WiFi sleep is disabled (already implemented)
- **mDNS not resolving**: Wait 2-3 seconds after WiFi connection, check firewall
- **Serial corruption on ESP32**: LED operations can cause this - UART restoration is implemented

### OTA Update Issues
- **Update fails**: Check GitHub token validity, ensure binary name matches
- **Timeout during update**: Watchdog feeding is now implemented
- **MITM security**: Currently using `setInsecure()` - see Security Considerations above
- **ESP8266 upload fails**: Use the separate server at `http://<ip>:8888/`

### Boot Loop
- **Device keeps restarting**: After 5+ quick restarts, enters boot loop mode
- **Clear boot loop**: Access `/invalidateConfig` to reset
- **Debugging**: Check `/logsStream` for WebSocket-based real-time logs

## Code Quality Standards

### Error Logging
- **Always use `LOG_PRINTLN`** instead of `Serial.println` for errors
- Errors should go to both Serial and WebSocket for visibility
- Use `DEBUG_PRINTLN` for development-only messages

### Platform Differences
- Always use `#ifdef ESP32` / `#elif defined(ESP8266)` guards
- Test platform-specific APIs separately
- Document any platform limitations in comments

### Resource Management
- Close connections explicitly (`client.stop()`)
- Free allocated memory (`free()`, `delete`)
- Feed watchdog during long operations
- Validate inputs before processing

## Future Enhancements

Potential improvements for production deployments:
1. **OTA Security**: Implement certificate validation (see Security Considerations)
2. **Firmware Verification**: Add cryptographic signature checking
3. **Rollback Capability**: ESP32 partition management for safe updates
4. **WiFi Event Handlers**: Better state management with event callbacks
5. **HTTP Authentication**: Add basic auth for configuration pages
6. **Rate Limiting**: Prevent brute-force attacks on config endpoints
7. **Metrics Export**: Prometheus/InfluxDB integration for monitoring
