#include "ota_handler.h"
#include "globals.h"

#include <ArduinoJson.h>

#ifdef ESP32
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFi.h>
#include <esp_task_wdt.h>
#elif defined(ESP8266)
#include "esp8266_ota_update.h"

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WiFi.h>
#endif

uint32_t checkForSoftwareUpdateMillis = 60 * 60 * 1000; // check for software update every 1 hour
uint64_t lastCheckForUpdateMillis = 0;

WiFiClientSecure getSecureClient()
{
    WiFiClientSecure secureClient;
    secureClient.setInsecure(); // Skip certificate verification

#ifdef ESP32
    httpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS); // for some reason github redirects all the time (throws a 302)
#elif defined(ESP8266)
    ESPhttpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS); // for some reason github redirects all the time (throws a 302))
#endif
    return secureClient;
}

void ESPGithubOtaUpdate::getLatestReleaseInfo(String &version, String &updateURL)
{
    // Initialize return values
    version = "0.0.0";
    updateURL = "";

    WiFiClientSecure secureClient = getSecureClient();
    HTTPClient httpClient;

    // RAII-style cleanup guard to ensure httpClient.end() is always called
    struct HTTPClientGuard {
        HTTPClient& client;
        HTTPClientGuard(HTTPClient& c) : client(c) {}
        ~HTTPClientGuard() { client.end(); }
    } guard(httpClient);

    String url = String(apiEndpoint) + "/repos/" + releaseRepo + "/releases/latest";
    String payload;

    DEBUG_PRINTLN(String("Requesting ") + url);

    if (!httpClient.begin(secureClient, url))
    {
        LOG_PRINTLN(F("OTA: Failed to begin HTTP connection"));
        return; // Guard will cleanup
    }

    httpClient.setTimeout(30000);  // 30 second timeout
    httpClient.setConnectTimeout(10000);  // 10 second connect timeout
    httpClient.addHeader("Authorization", String("token ") + authToken);
    httpClient.addHeader("Accept", "application/vnd.github.v3+json");
    httpClient.addHeader("User-Agent", "ESPGithubOtaUpdate/1.0");
    int httpCode = httpClient.GET();

    if (httpCode == HTTP_CODE_UNAUTHORIZED)
    {
        if (strlen(authToken) == 0)
            LOG_PRINTLN(F("OTA: 401 Unauthorized - GitHub token is empty"));
        else
            LOG_PRINTLN(F("OTA: 401 Unauthorized - Check GitHub token validity"));
        return; // Guard will cleanup
    }

    DEBUG_PRINT("OTA Update: got HTTP code ");
    DEBUG_PRINTLN(String(httpCode));

    if (httpCode != HTTP_CODE_OK)
    {
        return; // Guard will cleanup
    }

    DEBUG_PRINTLN(String("Got response from ") + url);
    payload = httpClient.getString();

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload.c_str());

    if (error)
    {
        LOG_PRINTLN("OTA: Failed to parse JSON response");
        return; // Guard will cleanup
    }

    const char *tagName = doc["tag_name"];
    if (!tagName)
    {
        LOG_PRINTLN("OTA: No tag_name in response");
        return; // Guard will cleanup
    }

    JsonArray assets = doc["assets"];
    for (auto value : assets)
    {
        JsonObject asset = value.as<JsonObject>();
        const char *name = asset["name"];
        if (String(name) == binaryFileName)
        {
            const char *browserDownloadUrl = asset["browser_download_url"];
            DEBUG_PRINT("OTA Update: found download URL: ");
            DEBUG_PRINTLN(browserDownloadUrl);
            version = String(tagName);
            updateURL = String(browserDownloadUrl);
            break;
        }
    }

    // Guard destructor will call httpClient.end()
}

bool ESPGithubOtaUpdate::isNewerVersionAvailable(String &latestVersion, String &updateURL)
{
    getLatestReleaseInfo(latestVersion, updateURL);
    int currentMajor, currentMinor, currentPatch;
    int latestMajor, latestMinor, latestPatch;
    sscanf(currentVersion, "%d.%d.%d", &currentMajor, &currentMinor, &currentPatch);
    sscanf(latestVersion.c_str(), "%d.%d.%d", &latestMajor, &latestMinor, &latestPatch);
    bool newer_firmware = (latestMajor > currentMajor) ||
                          (latestMajor == currentMajor && latestMinor > currentMinor) ||
                          (latestMajor == currentMajor && latestMinor == currentMinor && latestPatch > currentPatch);
    if (newer_firmware)
    {
        DEBUG_PRINT("Found new firmware at ");
        DEBUG_PRINTLN(updateURL);
    }

    return newer_firmware;
}

ESPGithubOtaUpdate::ESPGithubOtaUpdate(const char *v, const char *b, const char *r, const char *a, const char *endpoint) : currentVersion(v), binaryFileName(b), releaseRepo(r), authToken(a), apiEndpoint(endpoint)
{
#ifdef ESP8266
    setupEsp8266OtaUpdate();
#endif

    isInited = true;
}

void ESPGithubOtaUpdate::upgradeSoftware()
{
    if (!isInited)
    {
        DEBUG_PRINTLN(F("OTA Updater not inited. Exiting"));
        return;
    }
    String latestVersion;
    String updateURL;
    if (isNewerVersionAvailable(latestVersion, updateURL) && updateURL.length() > 0)
    {
        upgradeSoftware(updateURL.c_str());
    }
    else
    {
        LOG_PRINTLN("OTA: No new firmware available");
    }
}

void ESPGithubOtaUpdate::upgradeSoftware(const char *updateURL)
{
    if (!isInited || updateURL == nullptr || strlen(updateURL) == 0)
    {
        LOG_PRINTLN("OTA: Handler not initialized or invalid update URL");
        return;
    }

    // Check WiFi connection before attempting update
    if (WiFi.status() != WL_CONNECTED)
    {
        LOG_PRINTLN("OTA: WiFi not connected, aborting update");
        return;
    }

    WiFiClientSecure secureClient = getSecureClient();

#ifdef ESP32
    // Feed watchdog and setup progress callback for ESP32
    esp_task_wdt_reset();
    httpUpdate.onProgress([](int current, int total) {
        esp_task_wdt_reset();  // Feed watchdog during update
        if (current % (total / 10) == 0) {  // Print every 10%
            Serial.printf("OTA Progress: %d%%\n", (current * 100) / total);
        }
    });
    t_httpUpdate_return ret = httpUpdate.update(secureClient, updateURL, currentVersion);
#elif defined(ESP8266)
    // Feed watchdog and setup progress callback for ESP8266
    ESP.wdtFeed();
    ESPhttpUpdate.onProgress([](int current, int total) {
        ESP.wdtFeed();  // Feed watchdog during update
        if (current % (total / 10) == 0) {  // Print every 10%
            Serial.printf("OTA Progress: %d%%\n", (current * 100) / total);
        }
    });
    t_httpUpdate_return ret = ESPhttpUpdate.update(secureClient, updateURL, currentVersion);
#endif

    if (ret == HTTP_UPDATE_OK)
    {
        LOG_PRINTLN("OTA: Update successful, rebooting...");
        ESP.restart();
    }
    else
    {
        // If the update fails, print the error code and message
#ifdef ESP32
        LOG_PRINTLN("OTA: Update failed error (" + String(httpUpdate.getLastError()) + "): " + httpUpdate.getLastErrorString());
#elif defined(ESP8266)
        LOG_PRINTLN("OTA: Update failed error (" + String(ESPhttpUpdate.getLastError()) + "): " + ESPhttpUpdate.getLastErrorString());
#endif
    }
}

void ESPGithubOtaUpdate::checkForSoftwareUpdate()
{
#ifdef ESP8266
    handleEsp8266OtaUpdate();
#endif
    if (millis() - lastCheckForUpdateMillis > checkForSoftwareUpdateMillis)
    {
        lastCheckForUpdateMillis = millis();

        upgradeSoftware();
    }
}

void ESPGithubOtaUpdate::registerFirmwareUploadRoutes(AsyncWebServer *webServer, std::map<String, String> *routeDescriptions)
{
    if (!webServer)
        return;

#ifdef ESP8266
    // For some weird bug, can't use AsyncWebServerRequest with Esp8266 for upload.
    webServer->on("/uploadFirmware", HTTP_GET, [](AsyncWebServerRequest *request)
                  { request->redirect("http://" + WiFi.localIP().toString() + ":8888/"); });

#elif defined(ESP32)
    webServer->on("/uploadFirmware", HTTP_GET, [](AsyncWebServerRequest *request)
                  { request->send(200, "text/html", "<form method='POST' action='/firmwareUploadSave' enctype='multipart/form-data'>"
                                                    "<input type='file' name='firmware'>"
                                                    "<input type='submit' value='Upload Firmware'>"
                                                    "</form>"); });

    if (routeDescriptions != nullptr)
    {
        routeDescriptions->insert(std::make_pair("/uploadFirmware", "Upload firmware directly from the browser"));
    }

    webServer->on("/firmwareUploadSave", HTTP_POST, [](AsyncWebServerRequest *request) {}, // Placeholder for final response to the client, actual response will be sent in the upload handler
                  [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
                  {
        static bool uploadError = false;

        if (!index)
        {
            uploadError = false;
            LOG_PRINTLN("Firmware upload started: " + filename);

            // Validate filename
            if (!filename.endsWith(".bin"))
            {
                uploadError = true;
                request->send(400, "text/plain", "Invalid firmware file - must be .bin");
                return;
            }

            if (!Update.begin(UPDATE_SIZE_UNKNOWN))
            {
                Update.printError(Serial);
                request->send(500, "text/plain", "Update failed to start");
                uploadError = true;
                return;
            }
            LOG_PRINTLN("Firmware upload started successfully");
        }

        if (uploadError)
        {
            return; // Skip processing if error occurred
        }

        // Feed watchdog during upload
        #ifdef ESP32
            esp_task_wdt_reset();
        #elif defined(ESP8266)
            ESP.wdtFeed();
        #endif

        // Write received data to the update
        if (Update.write(data, len) != len)
        {
            Update.printError(Serial);
            request->send(500, "text/plain", "Update failed during write");
            uploadError = true;
            Update.abort();
            return;
        }

        if (final)
        {
            if (Update.end(true))
            {
                LOG_PRINTLN("Firmware upload complete: " + String(index + len) + " bytes");
                request->send(200, "text/plain", "Upload complete, device will restart.");
                delay(3000); // Short delay to ensure the response is sent before reboot
                ESP.restart();
            }
            else
            {
                Update.printError(Serial);
                request->send(500, "text/plain", "Update failed at end");
            }
        } });
#endif
}
