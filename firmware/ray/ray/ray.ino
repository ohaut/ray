
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#define MQTT_MAX_PACKET_SIZE 512
#include <PubSubClient.h>
#include <Ticker.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include "HTTPUpdateServer.h"

#include "LEDDimmers.h"
#include "ConfigMap.h"
#include "ray_global_defs.h"

#define DEVICE_TYPE "3CHANLED"

int led_pin = 13;

LEDDimmers dimmers;
extern ConfigMap configData;

ESP8266WebServer server(80);
HTTPUpdateServer upd_server;



void setupDimmers() {
  float boot_values[3];
  for (int i=0;i<3; i++)
    boot_values[i] = getDimmerStartupVal(i);
  /* switch on leds */
  dimmers.setup(boot_values);
}

bool wifi_connected;

void setup(void){

  /* start the serial port and switch on the PCB led */
  Serial.begin(115200);
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);


  /* read the configuration, and setup the HTTP config server */
  configServerSetup(&server);

  /* setup the dimmers, now that we have the configuration */
  setupDimmers();

  /* setup the HTTP API */
  setupHTTPApi(&server);

  /* setup the /update-app/ server for app.html.gz updating */
  upd_server.setup(&server);

  /* try to connect to the wifi, otherwise we will have an access point */
  wifi_connected = wifiSetup();

  /* configure the Over The Air firmware upgrades */
  ArduinoOTA.setHostname(configData["mqtt_id"]);
  ArduinoOTA.onStart([]() { dimmers.halt(); });
  ArduinoOTA.onError([](ota_error_t error) { dimmers.restart(); });
  ArduinoOTA.onEnd([](){
                     for (int i=0;i<30;i++){
                        analogWrite(led_pin,(i*100) % 1001);
                        delay(50);
                     }
                   });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    static bool toggle = true;
    toggle = !toggle;
    digitalWrite(led_pin, toggle);
  });
  ArduinoOTA.begin();


  SSDP_setup(&server, configData["mqtt_id"], "OHAUT Ray");


#if 1
  /* failsafe recovery during devel */
  for (int i=0;i<20; i++) {
      ArduinoOTA.handle();
      delay(100);
  }
#endif

  if (wifi_connected)
  {
    digitalWrite(led_pin, HIGH);
    setupMQTTHandling(DEVICE_TYPE);
  }

  server.begin();
 }

void loop(void){
  ArduinoOTA.handle();
  server.handleClient();
  if (wifi_connected)
     MQTTHandle();
}
