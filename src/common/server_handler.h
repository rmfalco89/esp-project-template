#ifndef SERVER_HANDLER_H
#define SERVER_HANDLER_H

void setupServer(bool config_mode);
void loopServer();

// Routes go here
void routeHome();
void rootReboot();
void routeConfigure();
void routeSaveConfiguration();
void routeInvaldateConfig();
void routeCheckUpdate();
void routeUploadFirmware();
void routeUploadFirmwareSave();

#endif // SERVER_HANDLER_H