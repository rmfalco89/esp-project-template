/* Project name: <project name>
  <project description>
*/

#include "Arduino.h"

#include "common/common_main.h"
#include "common/eeprom_utils.tpp"
#include "common/globals.h"

#include "globals.h"
#include "serverHandles.h"

/** Wiring **/
// <consider writing here the wiring scheme
void setup(void)
{
  commonSetup();
  addServerHandles();

  // project-specific handles, if any
  // addServerHandles();

  // project-specific eeprom config, if any
  // readConfigFromEeprom();

  // Init components

  LOG_PRINTLN("Full setup complete");
}

void loop(void)
{
  if (commonLoop() == 2) // don't run if in bootloop or if missing configuration
  {
    DEBUG_PRINTLN("bootloopMode, skipping main loop");
    delay(500);
    return;
  }

  if (!commonLoop() || systemConfiguration == nullptr)
  {
    delay(500);
    return;
  }
  // delay(1000);
  // LOG_PRINTLN("Loop cycle complete");
}
