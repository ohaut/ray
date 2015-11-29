#include "MQTTDevice.h"
#include <PubSubClient.h>


int reconnect_retry_time = 1000;
const int max_reconnect_retry_time = 60000*2;

MQTTDevice::MQTTDevice() {
    _client = new PubSubClient(_mqttClient);   
}

void MQTTDevice::setup(const char* server, const char *path, const char *client_id,
                       const char* user, const char* pass){

  _server = server;
  _path = path;    
  _client_id = client_id;
  _user = user;
  _pass = pass;
  _elements = NULL;     
  _last_reconnect = millis() - reconnect_retry_time;      
  setup();    
}
  
void MQTTDevice::subscribe(const char *name, SUBS_CALLBACK(fn)) {
  SubscribedElement** element=&_elements;

  while(*element!=NULL)
    element = &((*element)->next);
   
  *element = new SubscribedElement(name, fn);

  String path = _path;
  path += "/";
  path += name;

  Serial.printf("path:%s\r\n", path.c_str());
  _client->subscribe(path.c_str());
  
}


  
void MQTTDevice::publish(const char *name, const char *value) {
  String path = _getPathFor(name);
  _client->publish(path.c_str(), value, true);

  for (int i=0; i<20; i++) {
    _client->loop();
    delay(5);
  }
  
  path += "/state";
  _client->publish(path.c_str(), value, true);
  for (int i=0; i<20; i++) {
    _client->loop();
    delay(5);
  }
}

MQTTDevice *_singleton = NULL;

void mqtt_handle_message(char* topic, byte* payload, unsigned int length) {
  if (_singleton)
    _singleton->_handle_message(topic, payload, length);
}

void MQTTDevice::setup(){
  _singleton = this;
  _client->setServer(_server, 1883);
  _client->setCallback(mqtt_handle_message);
  
}

void MQTTDevice::_resubscribe() {
  for (SubscribedElement *element=_elements; element!=NULL; element=element->next) {
    _client->subscribe(_getPathFor(element->name).c_str());
  }
}

String MQTTDevice::_getPathFor(const char *name) {
  String path = _path;
  path += "/";
  path += name;
  return path;
}

void MQTTDevice::_handle_message(char* topic, byte* payload, unsigned int length) {

  for (SubscribedElement *element=_elements; element!=NULL; element=element->next) {
    if (_getPathFor(element->name) == topic) {
      element->callback(payload, length);
    }
  }
}

void MQTTDevice::handle() {
  _client->loop();
  
  if (!_client->connected()) {
    long now = millis();
    if (now - _last_reconnect > reconnect_retry_time)
    {
      _reconnect();
      _last_reconnect = now;
    }
  }
}


bool MQTTDevice::connected() {
  return _client->connected();
}

void MQTTDevice::_reconnect() {

  if (!_client->connected()) {
    Serial.printf("Attempting MQTT connection to %s ...", _server);
    // Attempt to connect
    if (_client->connect(_client_id, _getPathFor("led1/state").c_str(), MQTTQOS2, true, "0" )) {
        Serial.println(" connected!");
        _resubscribe();
    }
    else
    {
      reconnect_retry_time = reconnect_retry_time * 2;
      if (reconnect_retry_time > max_reconnect_retry_time)
        reconnect_retry_time = max_reconnect_retry_time;
      Serial.println(" failed :(");
    }
  }
}


