require('myconfig')

STA_AP = 'laotrainternet2'
STA_PASS = 'yavaledehackers33'

MQTT_SERVER = "192.168.1.251"
MQTT_PORT = 1883
MQTT_SECURE = 0
MQTT_USER = "user"
MQTT_PASS = "password"
MQTT_DEVICE_ID = MQTT_DEVICE_ID or ("ESP-" .. node.chipid())
MQTT_ROOM = MQTT_ROOM or "room"
MQTT_PATH ="/home/" .. MQTT_ROOM .. "/" .. MQTT_DEVICE_ID
MQTT_WIFI_CHECK_MS = 100
MQTT_RECONNECT_MS = 10000

AP_NAME = MQTT_ROOM .. "-" .. MQTT_DEVICE_ID
AP_PASS = STA_PASS or ('PASS-' .. node.chipid())

