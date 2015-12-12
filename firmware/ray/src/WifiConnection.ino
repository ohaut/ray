#include <ESP8266WiFi.h>
extern ConfigMap configData;
bool wifiSetup()
{
  int connect_tries=3;
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(configData["wifi_sta_ap"],
             configData["wifi_sta_pass"]);
  while(--connect_tries && WiFi.waitForConnectResult() != WL_CONNECTED){
    WiFi.begin(configData["wifi_sta_ap"],
               configData["wifi_sta_pass"]);
    Serial.println("WiFi failed, retrying.");
  }

  if (connect_tries <= 0) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(configData["wifi_ap_ssid"], configData["wifi_ap_pass"]);
    WiFi.begin();
    return false;
  }

  return true;
  
}

