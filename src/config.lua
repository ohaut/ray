STA_AP = 'laotrainternet2'
STA_PASS = 'yavaledehackers33'

AP_NAME = 'ESP-' .. node.chipid()
AP_PASS = 'PASS-' .. node.chipid()

MQTT_SERVER = "192.168.1.247"
MQTT_PORT = 1883
MQTT_SECURE = 0
MQTT_USER = "user"
MQTT_PASS = "password"
MQTT_DEVICE_ID = "ESP8266-" .. node.chipid()
MQTT_PATH ="/home/1/" .. MQTT_DEVICE_ID
MQTT_WIFI_CHECK_MS = 100
MQTT_RECONNECT_MS = 10000
