//
// This file is provided as a workaround for the >1.6.5
// Arduino environments which are uncapable of auto discovering
// function names in other .ino files for some reason
//

#ifndef __ray_global_defs_h
#define __ray_global_defs_h

float getDimmerStartupVal(int dimmer);
void configServerSetup(ESP8266WebServer *server);
void setupHTTPApi(ESP8266WebServer *server);
bool wifiSetup();
bool setupMQTTHandling();
void MQTTHandle();
void setDimmerAndPublish(int channel, int value);
void setDefaultConfig();
void setupDefaultConfigLamp();
void handleConfigLamp();
void handleConfigLampPost();

#endif
