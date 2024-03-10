# ESP32 new project Template

Getting up and running is extremely easy with the ESP32 new project template. With a minimal understanding of what the template offers, by modifying a few files you'll be able to run your code in no time.

To get started, open `main.cpp`, and set the following variables to fit your needs.
```
const char *SW_VERSION = "0.0.1";
const char *BINARY_NAME = "esp32devkitc.bin";
const char *releaseRepo = "<github_username>/<proj>";
const char *GITHUB_TOKEN = "<your_github_token>"; // Update with your GitHub token
const char *WIFI_SSID = "BellaMozzarella";
const char *WIFI_PASSWORD = "riccardo";
```

----------

## What you get for free
1. Watchdog
2. Wifi and Server setup
3. Configuration pages
4. EEPROM handling
5. OTA update

### Watchdog
Default 30 seconds, change it through `watchdog_timeout_s` in `config.cpp`

### Wifi and Server setup
Upon restart, the code checks whether a valid configuration was already set and if that was a quick restart.
If a valid configuration is not found in the EEPROM, or if it was a quick restart (a restart that happened in less than 30 seconds), the device enters in configuration mode for 5 minutes.  

#### Configuration mode
The device exposes a wifi network whose default SSID is `ArduinoNetConfig`. 

This allows the user to set a new configuration by navigating to `http://arduino.local/configure`.  
Once a valid configuration is provided, the device saves it to the EEPROM, and exits configuration mode.  
A basic configuration consists in an SSID, a password, and a hostname (default `arduino`).

#### Run mode
In normal run mode, the device connects to the WiFi network defined in the configuration.  
On top of that, this template exposes default routes to
- modify the existing configuration
- invalidate the configuration, which in turn will force the device to enter in configuration mode
- reboot the device (`https://<hostname>/reboot`)

### Configuration pages
`device_configuration.h` contains structs that are automatically saved to the EEPROM.  
They are accessible through `http://<hostname>/configure`.


### EEPROM handling
This template takes care of saving the configuration to the EEPROM, and exposes the methods to save, retrieve and invalidate data also for new structs.

### OTA update
If github authentication info are set in `config.cpp`, the code will periodically check for updates on github.  
This works also for private repositories.  
It's possible to force-check for updates by navigating to `http://<hostname>/checkForUpdates`.  
It is also possible to upload a new firmware through the browser by navigating to `http://<hostname>/uploadFirmware`.

### Server default routes
- `/`: home
- `/reboot`: reboot
- `/configure`: configure
- `/invalidateConfig`: delete old configuration (forces config mode on restart)
- `/checkForUpdates`: checks for new firmware on github
- `/uploadFirmware`: allows upload of firmware via the browser
