
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <ESP8266WebServer.h>

#include "LEDDimmers.h"
#include "MQTTDevice.h"
#include "ConfigMap.h"


#define BOOT_DIMMER_VALUE 1.0

int led_pin = 13;

LEDDimmers dimmers;
MQTTDevice mqtt;
bool mqtt_enabled = false;

extern ConfigMap configData;

ESP8266WebServer server(80);

void fancy_blink(){
  for (int i=0;i<30;i++){
    analogWrite(led_pin,(i*100) % 1001);
    delay(50);
  }
}


void dimmer_status(int led, byte* payload, unsigned int length) {
  char str[4];

  if (length>3) 
    return;
    
  memset(str, 0, length);
  memcpy(str, payload, length);

  String value(str);
  dimmers.setDimmer(led, value.toFloat()/100.0);
  
}


void setDimmerAndPublish(int channel, int value) {

    dimmers.setDimmer(channel, (float)value/100.0);
    if (mqtt_enabled) {
      char  pub_name[8];
      char  pub_data[6];
      
      sprintf(pub_name, "led%d", channel+1);
      sprintf(pub_data, "%d", value);
      mqtt.publish(pub_name, pub_data);
    }
}

void handleSetLed() {

    int ch , val;
    char* str_ch = strdup(server.arg("ch").c_str());
    char* str_val = strdup(server.arg("val").c_str());

    if (!strlen(str_val)) { /* we asume no ch means all channels */
      server.send(422, "text/html", "val parameter missing on URL");
      goto _exit;
    }
    ch = atoi(str_ch);
    val = atoi(str_val);

    if (ch<0 || ch>=3) {
      server.send(422, "text/html", "ch out of range (0..2)");
      goto _exit;
    }

    if (val<0 || val>100) {
      server.send(422, "text/html", "val out of range (0..100)");
      goto _exit;
    }

    if (strlen(str_ch))
      setDimmerAndPublish(ch, val);
    else {
      for (ch=0; ch<N_DIMMERS; ch++)
        setDimmerAndPublish(ch, val);
    }
    server.send(200, "text/html", "channel set correctly");
 _exit:
    free (str_ch);
    free (str_val);
}




void setupMQTTHandling() {
  mqtt.setup(configData["mqtt_server"],
             configData["mqtt_path"],
             configData["mqtt_id"]);
            
  mqtt.setup();

  mqtt.subscribe("led1", [](byte *data, unsigned int length) {
                              dimmer_status(0, data, length); });
  mqtt.subscribe("led2", [](byte *data, unsigned int length) {
                              dimmer_status(1, data, length); });
  mqtt.subscribe("led3", [](byte *data, unsigned int length) {
                              dimmer_status(2, data, length); });

}

void setup(void){

  Serial.begin(115200);
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);

  configServerSetup(&server);

  
  

  /* switch on leds */

  float boot_values[] = { configData["startup_val_0"]? atoi(configData["startup_val_0"])/100.0 : 1.0,
                          configData["startup_val_1"]? atoi(configData["startup_val_1"])/100.0 : 1.0,
                          configData["startup_val_2"]? atoi(configData["startup_val_2"])/100.0 : 1.0 };
  
  dimmers.setup(boot_values);
  
  bool wifi_connected = wifiSetup();
  
  ArduinoOTA.setHostname(configData["mqtt_id"]);
  ArduinoOTA.onStart([]() { dimmers.halt(); });
  ArduinoOTA.onError([](ota_error_t error) { dimmers.restart(); }); 
  ArduinoOTA.onEnd(fancy_blink);
  ArduinoOTA.begin();

  /* failsafe recovery during devel */
  for (int i=0;i<20; i++)
  {
      ArduinoOTA.handle();
      delay(100);
  }

  mqtt_enabled = strlen(configData["mqtt_server"])>0;

  if (mqtt_enabled) 
    setupMQTTHandling();
  
  if (wifi_connected) 
    digitalWrite(led_pin, HIGH);
  
  server.on("/setLed", HTTP_GET,  handleSetLed);
  server.begin();

 }



void sendMQTTStartupValues() {
  static bool startup_values_sent = false;
  if (mqtt.connected() && !startup_values_sent)
  {
    for (int i=0; i<3; i++) {
      char cf_key[32], pub_name[8];
      const char *cf_data;
      sprintf(cf_key, "startup_val_l%d", i);
      sprintf(pub_name, "led%d", i+1);
      cf_data = configData[cf_key];
      if (cf_data && strlen(cf_data))
        mqtt.publish(pub_name, cf_data);
    }
    startup_values_sent = true;
  }
}



void loop(void){
  ArduinoOTA.handle();
  server.handleClient();
  if (mqtt_enabled)  {
    mqtt.handle();
    sendMQTTStartupValues();
  }
} 


