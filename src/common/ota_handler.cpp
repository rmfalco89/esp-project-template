#include "ota_handler.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFi.h>

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
#endif

uint32_t checkForSoftwareUpdateMillis = 60 * 60 * 1000; // check for software update every 1 hour
uint64_t lastCheckForUpdateMillis = 0;

void otaSetup(const char *version, const char *binaryFileName, const char *repo, const char *token)
{
    // This function can be used for initial setup if needed.
}

void ESP32_GithubOtaUpdate::getLatestReleaseInfo(char *&version, char *&updateURL)
{
    WiFiClientSecure secureClient;
    secureClient.setInsecure(); // Skip certificate verification

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

bool ESP32_GithubOtaUpdate::isNewerVersionAvailable(char *&latestVersion, char *&updateURL)
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

ESP32_GithubOtaUpdate::ESP32_GithubOtaUpdate(const char *v, const char *b, const char *r, const char *a) : currentVersion(v), binaryFileName(b), releaseRepo(r), authToken(a)
{
    isInited = true;
}

void ESP32_GithubOtaUpdate::upgradeSoftware()
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

void ESP32_GithubOtaUpdate::upgradeSoftware(const char *updateURL)
{
    if (!isInited || updateURL == nullptr || strlen(updateURL) == 0)
    {
        Serial.println("OTA-Handler not initiated or invalid update URL.");
        return;
    }

    WiFiClientSecure secureClient;
    secureClient.setInsecure(); // Skip certificate verification
    httpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS); // for some reason github redirects all the time (throws a 302)

    t_httpUpdate_return ret = httpUpdate.update(secureClient, updateURL, currentVersion);
    if (ret == HTTP_UPDATE_OK)
    {
        Serial.println("Update successfully completed. Rebooting...");
        ESP.restart();
    }
    else
    {
        // If the update fails, print the error code and message
        Serial.printf("HTTP Update failed error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
    }
}

void ESP32_GithubOtaUpdate::checkForSoftwareUpdate()
{
    if (millis() - lastCheckForUpdateMillis > checkForSoftwareUpdateMillis)
    {
        lastCheckForUpdateMillis = millis();

        upgradeSoftware();
    }
}

void ESP32_GithubOtaUpdate::registerFirmwareUploadRoutes(WebServer *webServer)
{
    if (!webServer)
        return; // Safety check

    // Handler for the file upload form
    webServer->on("/uploadFirmware", HTTP_GET, [webServer]()
                  {
        DEBUG_PRINTLN("routeUploadFirmware");
        String html = "<!DOCTYPE html><html><head><title>Upload Firmware</title></head><body>"
                      "<form method='post' action='/firmwareUploadSave' enctype='multipart/form-data'>"
                      "<input type='file' name='firmware'>"
                      "<input type='submit' value='Upload Firmware'>"
                      "</form>"
                      "</body></html>";
        webServer->send(200, "text/html", html); });

    // Handler for the actual file upload
    webServer->on(
        "/firmwareUploadSave", HTTP_POST, [webServer]()
        { webServer->send(200, "text/plain", "Upload complete. Device will restart."); },
        [webServer]()
        {
            DEBUG_PRINTLN("routeFirmwareUploadSave");
            HTTPUpload &upload = webServer->upload();
            if (upload.status == UPLOAD_FILE_START)
            {
                Serial.printf("Update: %s\n", upload.filename.c_str());
                // Start with max available size
                if (!Update.begin(UPDATE_SIZE_UNKNOWN))
                {
                    Update.printError(Serial);
                }
            }
            else if (upload.status == UPLOAD_FILE_WRITE)
            {
                // Write the received bytes to flash
                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                {
                    Update.printError(Serial);
                }
            }
            else if (upload.status == UPLOAD_FILE_END)
            {
                if (Update.end(true))
                { // True to set the size to the current progress
                    Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
                    ESP.restart();
                }
                else
                {
                    Update.printError(Serial);
                }
            }
        });
}