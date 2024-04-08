#ifndef DEVICE_CONFIGURATION_H
#define DEVICE_CONFIGURATION_H

#include "utils.h"

/*
  Data Structure Alignment:
  When structures are saved to and read from EEPROM,
  padding added by the compiler for alignment can cause issues.
  Using #pragma pack(push, 1) before your structs and
  #pragma pack(pop) after can ensure no extra padding is added,
  making the size predictable.
*/
#pragma pack(push, 1)

/**
 * These structs are stored in the EEPROM
 */
struct JustRestarted
{
  bool justRestarted;

  // needed to allocate space when reading from eeprom for permanent configuration
  JustRestarted() {}

  JustRestarted(bool justRestarted_)
  {
    justRestarted = justRestarted_;
  }
};

struct DeviceConfiguration
{
  char ssid[30];
  char password[24];
  char hostname[20];
  char deviceName[20];
  char githubAuthToken[100];

  DeviceConfiguration() {}

  DeviceConfiguration(const char *s, const char *pass, const char *hostn, const char *name, const char *githubT)
  {
    strncpy(ssid, s, sizeof(ssid));
    strncpy(password, pass, sizeof(password));
    strncpy(hostname, hostn, sizeof(hostname));
    strncpy(deviceName, name, sizeof(deviceName));
    strncpy(githubAuthToken, githubT, sizeof(githubAuthToken));
  }

  String toStr() const
  {
    String text = "## Device config: \n";
    text += "\nSsid: '";
    text += String(ssid);
    text += "'\nPass: '";
    text += stringMask(String(password), '*');
    text += "'\nHostname: '";
    text += String(hostname);
    text += "'\nDevice Name: '";
    text += String(deviceName);
    text += "'\nGithub token: '";
    text += stringMask(String(githubAuthToken), '*');
    text += "'\n----\n";
    return text;
  }

  void printToSerial();
};

#pragma pack(pop)

bool readDeviceConfigurationFromEeprom();
void saveDeviceConfigurationToEeprom();
void invalidateDeviceConfigurationOnEeprom();

bool readJustRestartedFromEeprom();
void saveJustRestartedToEeprom(bool is_quick_restart);

#endif