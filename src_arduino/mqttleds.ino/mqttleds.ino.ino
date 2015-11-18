
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>

#include "LEDDimmers.h"
#include "MQTTDevice.h"
#include "ConfigMap.h"


#define BOOT_DIMMER_VALUE 1.0

int led_pin = 13;

LEDDimmers dimmers;
MQTTDevice mqtt;

extern ConfigMap configData;

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

void setup(void){

  Serial.begin(115200);

  dimmers.setup();
  dimmers.setDimmer(0, BOOT_DIMMER_VALUE);
  dimmers.setDimmer(1, BOOT_DIMMER_VALUE);
  dimmers.setDimmer(2, BOOT_DIMMER_VALUE);
  
  /* switch on led */
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);

 

  configServerSetup();
  wifiSetup();
  
  ArduinoOTA.setHostname("Lamp18W-prod");
  ArduinoOTA.onStart([]() { dimmers.halt(); });
  ArduinoOTA.onError([](ota_error_t error) { dimmers.restart(); }); 
  ArduinoOTA.onEnd(fancy_blink);
  ArduinoOTA.begin();

  /* failsafe recovery during devel */
  for (int i=0;i<20; i++)
  {
      //dimmers.setDimmer(0, ((float)19-i)/50.0);
      ArduinoOTA.handle();
      delay(100);
  }
  
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
  
  digitalWrite(led_pin, HIGH);
  
 }



void sendStartupValues() {
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
  configServerHandle();
  mqtt.handle();
  sendStartupValues();
} 


