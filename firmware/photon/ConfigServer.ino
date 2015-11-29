#include <ESP8266WebServer.h>
#include <FS.h>
#include "ConfigMap.h"
#include <Ticker.h>

ConfigMap configData;

#define CONFIG_FILENAME "/config.tsv"

ESP8266WebServer* _server;

void handleIndex() {
  String form =
    R"(<html>
      <body>
       <h1>LED Dimmer</h1>
       <ul>
        <li> <a href="/config/">config</a> </li>
       </ul>
       </body>
      </html>)";
      
  _server->send(200, "text/html", form);

}


 const char* form_fields[]={"wifi_sta_ap", "wifi_sta_pass", "mqtt_server",
                             "mqtt_path", "mqtt_id", "startup_val_l0", 
                             "startup_val_l1", "startup_val_l2", "mqtt_out_path",
                             "wifi_ap_ssid", "wifi_ap_pass",
                             NULL};

void setDefaultConfig();

void setDefaultConfig() {
  char esp_id[32];

  // create an unique ID for the AP SSID and MQTT ID
  sprintf(esp_id, "MICRODIMMER_%08x", ESP.getChipId());
  configData.set("wifi_sta_ap", "nonet");
  configData.set("wifi_sta_pass", "nonet");

  configData.set("wifi_ap_ssid", esp_id);
  configData.set("wifi_ap_pass", "dimmer123456");
  configData.set("mqtt_server", "192.168.1.251");
  configData.set("mqtt_path", "/home/kitchen/lamp1");
  configData.set("mqtt_out_path", "/home/kitchen/lamp1/status");
  configData.set("mqtt_id", esp_id);

}

void configSetup() {
  
  setDefaultConfig();
  SPIFFS.begin();
  File configFile = SPIFFS.open(CONFIG_FILENAME, "r");
  

  /* tiny TSV parser, move into ConfigMap */
  String key="", value="";
  bool parsing_value=false;
  while (configFile.available()) {
    int val = configFile.read();
    if (val=='\n' && key.length()!=0) {
      configData.set(key.c_str(), value.c_str());
      key = "";
      value = "";
      parsing_value = false;
    } else if (val=='\t') {
      parsing_value = true;
    } else {
      if (parsing_value) value += (char)val;
      else               key += (char)val;
    }
  }

  configFile.close();
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
       <form action="/config/" method="POST">   
        <h2>Lamp startup settings (0-100 or empty to get it from net)</h2>
        <div><label>Channel 0</label><input name='startup_val_l0' value='$startup_val_l0'></div>
        <div><label>Channel 1</label><input name='startup_val_l1' value='$startup_val_l1'></div>
        <div><label>Channel 2</label><input name='startup_val_l2' value='$startup_val_l2'></div>
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
       </body>
      </html>)";
  for (int i; form_fields[i]; i++) {
    const char *form_field = form_fields[i];
    String var_name = "$";
    var_name += form_field;
    form.replace(var_name, configData[form_field]);
  }

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


/* move this into ConfigMap */
void saveConfig() {
  File configFile = SPIFFS.open(CONFIG_FILENAME, "w");
  for (int i=0; form_fields[i]; i++) {
    String str = form_fields[i];
    str += '\t';
    str += configData[form_fields[i]];
    str += '\n';
    configFile.write((const uint8_t*)str.c_str(), str.length());
  }
  configFile.close();
}

void handleConfigPost() {
  
  for (int i=0; form_fields[i]; i++) {
    char *str = strdup(_server->arg(form_fields[i]).c_str());
    urldecode(str);
    configData.set(form_fields[i], str);
    free(str);
  }
  saveConfig();
  handleConfig();
  for (int i=0;i<100;i++){
    yield();
    delay(10);
  }
  ESP.restart();
}


float getDimmerStartupVal(int dimmer) {
    char key[16];
    const char *val;
    sprintf(key, "startup_val_l%d", dimmer);
    val = configData[key];
    
    if (val && strlen(val)) return atoi(val)/100.0;
    else                    return 1.0;
}

void configServerSetup(ESP8266WebServer *server) {
  _server = server;
  configSetup();
 
  server->on("/", HTTP_GET, handleIndex);
  server->on("/config/", HTTP_GET,  handleConfig);
  server->on("/config/", HTTP_POST,  handleConfigPost);
  
}

