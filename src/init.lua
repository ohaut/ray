require('config')
require('wifi')
require('telnet')
require('mqtt_connection')
require('pin_mapping')

gpio.mode(PIR_PIN, gpio.INPUT, gpio.PULLUP)

function pir_cb(conn, topic, data)
    print ("pir handler: ".. data)
end

m = mqtt_connection({pir1=pir_cb})

function publish_state(sensor, state)
    function safe_publish(sensor, state)
        m:publish(MQTT_PATH .. "/" .. sensor .. "/state", state, 0,0)
    end
    pcall(safe_publish, sensor, state)
end

publish_state("pir1","NOTCONNECTED")

gpio.trig(PIR_PIN, "both",function (level)
   if (level==0) then
     publish_state("pir1", "OFF")
   else
     publish_state("pir1", "ON")
    end
end)

