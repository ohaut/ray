#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#include "ConfigMap.h"
#include "version.h"

#include "ray_global_defs.h"

extern ConfigMap configData;

void handleSetLed() {

    int ch , val;
    char* str_ch = strdup(server.arg("ch").c_str());
    char* str_val = strdup(server.arg("val").c_str());

    if (!strlen(str_val)) { /* we asume no ch means all channels */
      server.send(422, "application/json", "{\"result\": \"1\", \"message:\": "
                                            "\"val parameter missing on URL\"}");
      goto _exit;
    }
    ch = atoi(str_ch);
    val = atoi(str_val);

    if (ch<0 || ch>=3) {
      server.send(422, "application/json", "{\"result\": \"2\", \"message:\": "
                                            "\"ch out of range (0..2)\"}");
      goto _exit;
    }

    if (val<0 || val>100) {
      server.send(422,  "application/json", "{\"result\": \"3\", \"message:\": "
                                            "\"val out of range (0..100)\"}");
      goto _exit;
    }

    if (strlen(str_ch))
      setDimmerAndPublish(ch, val);
    else {
      for (ch=0; ch<N_DIMMERS; ch++)
        setDimmerAndPublish(ch, val);
    }
    server.send(200, "application/json", "{\"result\": \"0\", \"message:\": "
                                          "\"channel set correctly\"}");
 _exit:
    free (str_ch);
    free (str_val);
}

int update_status = 0;

String getSPIFFSversion() {
    bool waiting_first_comma = true;
    bool reading_version = false;
    String result = "";
    File versionFile = SPIFFS.open("/VERSION_H", "r");

    if (!versionFile) goto error;

    while (versionFile.available()) {
      int val = versionFile.read();
      if (((char)val == '"') && reading_version) {
        versionFile.close();
        return result;
      }
      if (((char)val == '"') && waiting_first_comma) {
        reading_version = true;
        waiting_first_comma = false;
        continue;
      }
      if (reading_version) {
        result += (char) val;
      }
    }
    versionFile.close();
error:
    result = "NOTFOUND";
    return result;
}

void handleGetUpdateStatus()
{
  char result[128];
  String spiffs_version = getSPIFFSversion();

  sprintf(result, "{\"update_status\": \"%d\", "
                  "\"firmware_version\": \"%s\", "
                  "\"spiffs_version\": \"%s\"}",
          update_status, VERSION, spiffs_version.c_str());

  server.send(200, "application/json", result);
}

void handleUpdateSPIFFS() {
  Serial.println("Updating SPIFFS from HTTP");
  server.send(200, "application/json", "{\"result\": \"0\", \"message:\": "
                                          "\"SPIFFS updating, please wait\"}");
  server.stop();
  update_status = 1;
  t_httpUpdate_return ret = ESPhttpUpdate.updateSpiffs(
    "http://ohaut.org/ray/firmware/master/spiffs.bin");
  if (ret == HTTP_UPDATE_OK) update_status = 2;
  else update_status = -1;
  Serial.printf("Updating SPIFFS done: %d", update_status);
  configData.writeTSV(CONFIG_FILENAME);
  server.begin();
}
void handleUpdateFirmware() {
  Serial.println("Updating Firwmare from HTTP");
  server.send(200, "application/json", "{\"result\": \"0\", \"message:\": "
                                          "\"Firmware updating, please wait\"}");

  server.stop();
  update_status = 1;
  t_httpUpdate_return ret = ESPhttpUpdate.update(
    "http://ohaut.org/ray/firmware/master/firmware.bin");
  if (ret == HTTP_UPDATE_OK) update_status = 3;
  else update_status = -1;
  Serial.printf("Updating Firrmware done: %d", update_status);
  server.begin();
}

void handleUpdateAll() {
  Serial.println("Updating ALL:");
  server.send(200, "application/json", "{\"result\": \"0\", \"message:\": "
                                          "\"Updating everything, please wait\"}");

  server.stop();
  Serial.println(" * SPIFFS from HTTP");

  update_status = 1;
  t_httpUpdate_return ret = ESPhttpUpdate.updateSpiffs(
    "http://ohaut.org/ray/firmware/master/spiffs.bin");
  if (ret == HTTP_UPDATE_OK) update_status = 2;
  else update_status = -1;
  configData.writeTSV(CONFIG_FILENAME);

  if (update_status<0) {
    server.begin();
    return;
  }
  update_status = 1;

  Serial.println(" * Firmware from HTTP");

  ret = ESPhttpUpdate.update(
    "http://ohaut.org/ray/firmware/master/firmware.bin");
  if (ret == HTTP_UPDATE_OK) update_status = 3;
  else update_status = -1;
  Serial.printf("Updating Firmware done: %d", update_status);
  server.begin();
}

void setupHTTPApi(ESP8266WebServer *server) {
  server->on("/setLed", HTTP_GET,  handleSetLed);
  server->on("/update/status", HTTP_GET,  handleGetUpdateStatus);
  server->on("/update/spiffs", HTTP_GET,  handleUpdateSPIFFS);
  server->on("/update/firmware", HTTP_GET,  handleUpdateFirmware);
  server->on("/update/all", HTTP_GET,  handleUpdateAll);
}
