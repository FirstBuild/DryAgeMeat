/*
* Project DryAgeMeat
* Description:
* Author:
* Date:
*/

#include "DryAgeMeat.h"

DryAgeMeat dryAgeMeat;

SYSTEM_THREAD(ENABLED);
STARTUP(WiFi.selectAntenna(ANT_AUTO));

void setup(void) {
  dryAgeMeat.setup();
}

void loop(void) {
  dryAgeMeat.loop();
}
