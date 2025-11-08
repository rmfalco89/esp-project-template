#ifndef OTA_HANDLER_H
#define OTA_HANDLER_H

#include <Arduino.h>
#include <map>
#include <ESPAsyncWebServer.h>

class ESPGithubOtaUpdate
{
private:
    bool isInited = false;
    const char *currentVersion;
    const char *binaryFileName;
    const char *releaseRepo;
    const char *authToken;
    const char *apiEndpoint;

    void getLatestReleaseInfo(String &version, String &updateURL);
    bool isNewerVersionAvailable(String &latestVersion, String &updateURL);

public:
    ESPGithubOtaUpdate(const char *, const char *, const char *, const char *, const char * = "https://api.github.com");
    void checkForSoftwareUpdate();
    void upgradeSoftware();
    void upgradeSoftware(const char *);
    void registerFirmwareUploadRoutes(AsyncWebServer *, std::map<String, String> * = nullptr);
};

#endif
