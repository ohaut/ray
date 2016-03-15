#include <ESP8266WebServer.h>
#include <FS.h>
#include "ConfigMap.h"
#include <Ticker.h>
#include <functional>
#include "ray_global_defs.h"
#include "consts.h"
#ifdef SPIFFLESS
  #include "../data.h"
#endif
ConfigMap configData;


ESP8266WebServer* _server;

void handleIndex() {
  String form =
    R"(<html>
      <body>
       <h1>LED Dimmer</h1>
       <ul>
        <li> <a href="/config/connection/">connection config</a> </li>
        <li> <a href="/config/lamp/">lamp config</a> </li>
       </ul>
       </body>
      </html>)";

  _server->send(200, "text/html", form);

}

void setDefaultConfig() {
  char esp_id[32];

  // create an unique ID for the AP SSID and MQTT ID
  sprintf(esp_id, "MICRODIMMER_%08x", ESP.getChipId());
  configData.set("wifi_sta_ap", DEFAULT_STA_AP);
  configData.set("wifi_sta_pass", DEFAULT_STA_PASS);
  configData.set("mode", "lamp");
  configData.set("wifi_ap_ssid", esp_id);
  configData.set("wifi_ap_pass", "dimmer123456");
  configData.set("mqtt_server", "192.168.1.251");
  configData.set("mqtt_path", "/home/kitchen/lamp1");
  configData.set("mqtt_out_path", "/home/kitchen/lamp1/status");
  configData.set("mqtt_id", esp_id);

  // TODO(mangelajo): we could make this a bit modular
  setupDefaultConfigLamp();

}

void configSetup() {
  SPIFFS.begin();
  setDefaultConfig();
  configData.readTSV(CONFIG_FILENAME);
}

void handleConfig() {
  String form =
    R"(<html>
      <head>
        <style>
          label { float:left; padding-left:2em; width: 12em; }
         </style>
      <body>
       <h1>Configuration</h1>
       <form action="/config/connection/" method="POST">
        <h2>Dimmer mode</h2>
        <div><label>Mode</label>$mode_select</div>

        <h2>WiFi settings</h2>
        <div><label>WiFi ssid:</label><input name='wifi_sta_ap' value='$wifi_sta_ap'></div>
        <div><label>WiFi password:</label><input name='wifi_sta_pass' type='password' value='$wifi_sta_pass'></div>
        <h3>WiFi AP (fallback config)</h3>
        <div><label>AP ssid:</label><input name='wifi_ap_ssid' value='$wifi_ap_ssid'></div>
        <div><label>AP password:</label><input name='wifi_ap_pass' type='password' value='$wifi_ap_pass'></div>

        <h2>MQTT settings</h2>
        <div><label>MQTT Server:</label><input name='mqtt_server' value='$mqtt_server'></div>
        <div><label>MQTT (in) path:</label><input name='mqtt_path' value='$mqtt_path'></div>
        <div><label>MQTT Device ID:</label><input name='mqtt_id' value='$mqtt_id'></div>
        <div><label>MQTT (out) path:</label><input name='mqtt_out_path' value='$mqtt_out_path'></div>

          <input type="submit" value="Save and Reboot">
       </form>
         My WiFi STA IP: $IP
       </body>
      </html>)";

  String mode_select = "<select name='mode'>";
  char* mode = configData["mode"];
  mode_select += "<option value='lamp' ";
  if (strcmp(mode, "lamp") == 0)
    mode_select += "selected";
  mode_select += ">Lamp</option>";

  mode_select += "<option value='greenhouse'";
  if (strcmp(mode, "greenhouse") == 0)
    mode_select += "selected";
  mode_select += ">Greenhouse</option>";

  mode_select += "</select>";


  form.replace("$mode_select", mode_select);

  configData.replaceVars(form);

  form.replace("$IP", WiFi.localIP().toString());

  _server->send(200, "text/html", form);
}

// convert a single hex digit character to its integer value
unsigned char h2int(char c)
{
    if (c >= '0' && c <='9'){
        return((unsigned char)c - '0');
    }
    if (c >= 'a' && c <='f'){
        return((unsigned char)c - 'a' + 10);
    }
    if (c >= 'A' && c <='F'){
        return((unsigned char)c - 'A' + 10);
    }
    return(0);
}
void urldecode(char *urlbuf)
{
    char c;
    char *dst;
    dst=urlbuf;
    while ((c = *urlbuf)) {
        if (c == '+') c = ' ';
        if (c == '%') {
            urlbuf++;
            c = *urlbuf;
            urlbuf++;
            c = (h2int(c) << 4) | h2int(*urlbuf);
        }
        *dst = c;
        dst++;
        urlbuf++;
    }
    *dst = '\0';
}


void handlePost(std::function<void()> render_form_function) {
  /* Grab every known config entry from the POST request arguments,
   * urldecode it, and set it back to the config object.
   */
  configData.foreach(
      [](const char* key, const char* value, bool last) {
        if (_server->hasArg(key)) {
          const char *_str = _server->arg(key).c_str();
          if (_str) {
            char *str = strdup(_str);
            urldecode(str);
            configData.set(key, str);
            free(str);
          }
        }
      }
  );

  /* write a TSV file */
  configData.writeTSV(CONFIG_FILENAME);

  /* return the form again with the new data */
  render_form_function();

  /* allow for the data to be sent back to browser */
  for (int i=0;i<100;i++){
    yield();
    delay(10);
  }
  ESP.restart();
}

void handleConfigPost() {
  handlePost(handleConfig);
}

float getDimmerStartupVal(int dimmer) {
    char key[16];
    const char *val;
    sprintf(key, "startup_val_l%d", dimmer);
    val = configData[key];

    if (val && strlen(val)) return atoi(val)/100.0;
    else                    return 1.0;
}
#ifdef SPIFLESS
int32_t spiffs_hal_read(uint32_t addr, uint32_t size, uint8_t *dst);

class ConstReader {
  char _name[128];
  int _len;
  int _pos;
  PGM_P p;
public:
  ConstReader(const char *name, PGM_P data, int len)
  {
    p = data;
    strncpy(_name, name, 128);
    _len = len;
    _pos = 0;
    Serial.printf(">>0x%x\r\n", data);
  }

  int size() { return _len; }
  char* name() { return _name; } // use char* or String(x->name()) won't
                                        // work
  int available() { return _len - _pos; }
  int read(uint8_t* obuf, int len) {
    if (len>available()) len = available();
    //memcpy(obuf, p, len);
    //spiffs_hal_read(((uint32_t)p)-0x40200000, len, obuf);
    ESP.flashRead(((uint32_t)p)-0x40200000, (uint32_t*)obuf, len);
    //spiffs_hal_read(((uint32_t)p)-0x40200000, len, obuf);
    p += len;
    return len;
  }

};

bool spifless_exists(String s_name) {
  int i=0;
  const char* name = s_name.c_str();
  if (name[0]=='/') name++;
  while(spifless_names[i]) {
    if (strcmp(name, spifless_names[i]) == 0) {
      //Serial.printf("exists: [%s]\r\n", name);
      return true;
    }
    i++;
  }
  return false;
}

ConstReader *spifless_open(String s_name)
{
  int i=0;
  const char* name = s_name.c_str();
  if (name[0]=='/') name++;
  while(spifless_names[i]) {
    if (strcmp(name, spifless_names[i]) == 0) {
      return new ConstReader(name, spifless_datas[i], spifless_lengths[i]);
    }
    i++;
  }
  return NULL;
}
#endif


//format bytes
String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(ESP8266WebServer *server, String path){
  /*if (path.endsWith("config.tsv"))
  {
    server->send(403, "text/plain", "Unauthorized");
    return false;
  }*/
  if(path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
#ifndef SPIFLESS
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
#else
  if (spifless_exists(pathWithGz) || spifless_exists(path)) {
    if (spifless_exists(pathWithGz))
#endif
      path += ".gz";

#ifndef SPIFLESS
    File file = SPIFFS.open(path, "r");
    size_t sent = server->streamFile(file, contentType);
    file.close();
#else
    ConstReader *file = spifless_open(path);

    size_t sent = server->streamFile(*file, contentType);
    delete file;
#endif
    return true;
  }
  return false;
}

void handleConfigGet() {
  String json = "{";
  configData.foreach([&json](const char* key, const char* value, bool last) {
    json += "\"";
    json += key;
    json += "\": \"";
    json += value;
    json += "\"";
    if (!last) json += ",\n ";
  });
  json += "}";
  _server->send(200, "text/html", json);
}

void handleReboot() {
  _server->send(200, "text/html", "{\"result\": \"0\","
                                   "\"message\": \"rebooting\"}");
  /* allow for the data to be sent back to browser */
  delay(1000);
  ESP.restart();
}


void configServerSetup(ESP8266WebServer *server) {
  _server = server;
  configSetup();

  server->on("/", HTTP_GET, [server](){
    if(!handleFileRead(server, "/app.html")) {
      handleIndex();
    };
  });
  server->on("/config", HTTP_GET, handleConfigGet);
  server->on("/config", HTTP_POST, handleConfigPost);
  server->on("/reboot", HTTP_GET, handleReboot);

  // static file serving
  server->onNotFound([server](){
     if(!handleFileRead(server, server->uri()))
    server->send(404, "text/plain", "FileNotFound");
   });
}
