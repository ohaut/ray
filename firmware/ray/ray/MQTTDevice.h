#ifndef __MQTT_DEVICE_H
#define __MQTT_DEVICE_H

#define SUBS_CALLBACK(callback) void (*callback)(byte* value, unsigned int length)

class SubscribedElement;
class SubscribedElement {
  public:
  char *name;
  SUBS_CALLBACK(callback);
  SubscribedElement* next;
  SubscribedElement(const char *name, SUBS_CALLBACK(fn)) {
    this->name = strdup(name);
    this->callback = fn;
    this->next = NULL;
  }
  ~SubscribedElement() {
    free(name);
  }
};


class MQTTDevice {
private:
  const char *_path;
  const char *_server;
  const char *_user;
  const char *_pass;
  const char *_client_id;
  long _last_reconnect;
  SubscribedElement* _elements;

  WiFiClient _mqttClient;
  PubSubClient *_client;
  void _reconnect();
  void _resubscribe();
  String _getPathFor(const char* name);
public:
  MQTTDevice();
  void setup(const char* server, const char *path, const char *client_id,
             const char* user=NULL, const char* pass=NULL);
  void setup();
  void handle();
  void subscribe(const char *name, SUBS_CALLBACK(fn));
  void publish(const char *name, const char *value);
  void publishPath(const char *path, const char *value);
  void _handle_message(char* topic, byte* payload, unsigned int length);
  bool connected();
};

#endif
