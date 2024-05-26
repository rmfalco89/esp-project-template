#include "ota_handler.h"

#include <ArduinoJson.h>

#ifdef ESP32
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WiFi.h>
#endif

#ifndef DEBUG_PRINT
#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(str)   \
    {                      \
        Serial.print(str); \
    }
#define DEBUG_PRINTLN(str)   \
    {                        \
        Serial.println(str); \
    }
#else
#define DEBUG_PRINT(str)
#define DEBUG_PRINTLN(str)
#endif // #ifdef DEBUG
#endif // #ifndef DEBUG_PRINT

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

void otaSetup(const char *version, const char *binaryFileName, const char *repo, const char *token)
{
    // This function can be used for initial setup if needed.
}

void ESPGithubOtaUpdate::getLatestReleaseInfo(char *&version, char *&updateURL)
{
    WiFiClientSecure secureClient = getSecureClient();
    HTTPClient httpClient;

    String url = String("https://api.github.com/repos/") + releaseRepo + "/releases/latest";
    String payload;

    DEBUG_PRINTLN(String("Requesting ") + url);
    httpClient.begin(secureClient, url);
    httpClient.addHeader("Authorization", String("token ") + authToken);
    int httpCode = httpClient.GET();

    if (httpCode == HTTP_CODE_UNAUTHORIZED)
    {
        if (strlen(authToken) == 0)
            Serial.println(F("Got 401 Unauthorized, and github token is empty. Check your configuration"));

        else
            Serial.println(F("Got 401 Unauthorized. Check if your github token is valid and not expired."));
    }

    DEBUG_PRINT("OTA Update: got code ");
    DEBUG_PRINTLN(String(httpCode));

    if (httpCode == HTTP_CODE_OK)
    {
        DEBUG_PRINTLN(String("Got response from ") + url);
        payload = httpClient.getString();

        JsonDocument doc;
        deserializeJson(doc, payload.c_str());

        const char *tagName = doc["tag_name"];
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
                version = strdup(tagName);
                updateURL = strdup(browserDownloadUrl);
                break;
            }
        }
    }
    else
    {
        version = strdup("0.0.0");
        updateURL = nullptr;
    }

    httpClient.end();
}

bool ESPGithubOtaUpdate::isNewerVersionAvailable(char *&latestVersion, char *&updateURL)
{
    getLatestReleaseInfo(latestVersion, updateURL);
    int currentMajor, currentMinor, currentPatch;
    int latestMajor, latestMinor, latestPatch;
    sscanf(currentVersion, "%d.%d.%d", &currentMajor, &currentMinor, &currentPatch);
    sscanf(latestVersion, "%d.%d.%d", &latestMajor, &latestMinor, &latestPatch);
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

ESPGithubOtaUpdate::ESPGithubOtaUpdate(const char *v, const char *b, const char *r, const char *a) : currentVersion(v), binaryFileName(b), releaseRepo(r), authToken(a)
{
    isInited = true;
}

void ESPGithubOtaUpdate::upgradeSoftware()
{
    if (!isInited)
    {
        DEBUG_PRINTLN(F("OTA Updater not inited. Exiting"));
        return;
    }
    char *latestVersion = nullptr;
    char *updateURL = nullptr;
    if (isNewerVersionAvailable(latestVersion, updateURL) && updateURL != nullptr)
    {
        upgradeSoftware(updateURL);
    }
    else
    {
        Serial.println("Couldn't find new firmware");
    }
    if (latestVersion)
        free(latestVersion);
    if (updateURL)
        free(updateURL);
}

void ESPGithubOtaUpdate::upgradeSoftware(const char *updateURL)
{
    if (!isInited || updateURL == nullptr || strlen(updateURL) == 0)
    {
        Serial.println("OTA-Handler not initiated or invalid update URL.");
        return;
    }

    WiFiClientSecure secureClient = getSecureClient();

#ifdef ESP32
    t_httpUpdate_return ret = httpUpdate.update(secureClient, updateURL, currentVersion);
#elif defined(ESP8266)
    t_httpUpdate_return ret = ESPhttpUpdate.update(secureClient, updateURL, currentVersion);
#endif

    if (ret == HTTP_UPDATE_OK)
    {
        Serial.println("Update successfully completed. Rebooting...");
        ESP.restart();
    }
    else
    {
        // If the update fails, print the error code and message
#ifdef ESP32
        Serial.printf("HTTP Update failed error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
#elif defined(ESP8266)
        Serial.printf("HTTP Update failed error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
#endif
    }
}

void ESPGithubOtaUpdate::checkForSoftwareUpdate()
{
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

    webServer->on("/uploadFirmware", HTTP_GET, [webServer](AsyncWebServerRequest *request)
                  { request->send(200, "text/html", "<form method='POST' action='/firmwareUploadSave' enctype='multipart/form-data'>"
                                                    "<input type='file' name='firmware'>"
                                                    "<input type='submit' value='Upload Firmware'>"
                                                    "</form>"); });

    if (routeDescriptions != nullptr)
    {
        routeDescriptions->insert(std::make_pair("/uploadFirmware", "Upload firmware directly from the browser"));
    }

    webServer->on("/firmwareUploadSave", HTTP_POST, [webServer](AsyncWebServerRequest *request)
                  {
                      // Placeholder for final response to the client, actual response will be sent in the upload handler
                  },
                  [webServer](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
                  {
        if (!index)
        {
            Serial.printf("Update Start: %s\n", filename.c_str());

#ifdef ESP32
            if (!Update.begin(UPDATE_SIZE_UNKNOWN))
#elif defined(ESP8266)
            if (!Update.begin(1024 * 1024)) // Must give a size, let's use 1MB
#endif
{
                Update.printError(Serial);
            request->send(500, "text/plain", "Update failed at start");
            delay(2000);
            return;
        }
        }

        if (Update.write(data, len) != len) {
        Update.printError(Serial);
        request->send(500, "text/plain", "Update failed during write");
        delay(2000);
        return;
        }

        if (final) {
        if (Update.end(true))
        {
            Serial.printf("Update Success: %uB\n", index + len);
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
}