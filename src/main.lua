
gpio.mode(PUSHBUTTON_PIN, gpio.INPUT, gpio.PULLUP)

LED_VAL = {100, 100, 100}
LED_DIR = {  2,   2,   2}

function led_cb(led_n, data)
    if data=="ON" then
        set_dimmer(led_n,PWM_FREQ)
        LED_VAL[led_n] = 100
    elseif (data=="OFF") then
        set_dimmer(led_n,0)
        LED_VAL[led_n] = 0
    else
        LED_VAL[led_n] = tonumber(data)
        value = PWM_FREQ * tonumber(data)/100
        set_dimmer(led_n, value)
    end
end

function led1_cb(conn, topic, data) led_cb(1, data) end
function led2_cb(conn, topic, data) led_cb(2, data) end
function led3_cb(conn, topic, data) led_cb(3, data) end

m = mqtt_connection({led1=led1_cb,
                     led2=led2_cb,
                     led3=led3_cb})
function publish_state(sensor, state)
    function safe_publish(sensor, state)
        m:publish(MQTT_PATH .. "/" .. sensor .. "/state", state, 1,0)
    end
    result = pcall(safe_publish, sensor, state)
end

gpio.trig(PUSHBUTTON_PIN, "both",function (level)
   -- level doesn't seem to be reliable without debouncing
   tmr.delay(10000) -- debouncing
   if (gpio.read(PUSHBUTTON_PIN)==0) then
     publish_state("button", "ON")
     tmr.alarm(0, 50, 1,
        function()
            lv = LED_VAL[1] + LED_DIR[1] 
            if (lv<0) or (lv>100) then
                lv = LED_VAL[1] 
                tmr.stop(0)
            end
            led_cb(1, lv)
        end)
   else
     tmr.stop(0)
     publish_state("led1", LED_VAL[1])
     publish_state("button", "OFF")
     LED_DIR[1] = -LED_DIR[1] 
   end
end)


