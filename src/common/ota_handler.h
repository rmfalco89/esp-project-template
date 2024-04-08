#ifndef OTA_HANDLER_H
#define OTA_HANDLER_H

#include <Arduino.h>
#include <map>
#include <ESPAsyncWebServer.h>

void otaSetup(const char *, const char *, const char *, const char *);

class ESP32_GithubOtaUpdate
{
private:
    bool isInited = false;
    const char *currentVersion;
    const char *binaryFileName;
    const char *releaseRepo;
    const char *authToken;

    void getLatestReleaseInfo(char *&version, char *&updateURL);
    bool isNewerVersionAvailable(char *&latestVersion, char *&updateURL);

public:
    ESP32_GithubOtaUpdate(const char *, const char *, const char *, const char *);
    void checkForSoftwareUpdate();
    void upgradeSoftware();
    void upgradeSoftware(const char *);
    void registerFirmwareUploadRoutes(AsyncWebServer *, std::map<String, String> * = nullptr);
};

#endif
