
function mqtt_connection(subscriptions)
    m = mqtt.Client(MQTT_DEVICE_ID, 180, MQTT_USER, MQTT_PASS)
    m:lwt(MQTT_PATH .. "/lwt", "offline", 0, 0)

    function mqtt_subscribe()
      for subs, _ in pairs(subscriptions) do
        path = MQTT_PATH .. "/" .. subs
        m:subscribe(path , 2,
                    function(conn)
                    end)
        print("mqtt: subscribed " .. path)
       end
    end
    function connect()
          m:connect(MQTT_SERVER, 1883, MQTT_SECURE, function(conn)
            print("mqtt: connected to " .. MQTT_SERVER)
            tmr.stop(6)
            mqtt_subscribe() --run the subscription function
          end)
    end

    function reconnect()
        print ("mqtt: (re)connecting...")
        connect()
        tmr.alarm(6, MQTT_RECONNECT_MS, 1, connect)
    end

    function wait_for_wifi()
        -- initial connection timer
        tmr.alarm(6, MQTT_WIFI_CHECK_MS, 1,
          function()
              if wifi.sta.status() == 5 and wifi.sta.getip() ~= nil then
                tmr.stop(6)
                reconnect()
              end
          end
        )
    end

    m:on("offline",
        function()
            wifi.sta.disconnect()
            tmr.delay(1000000)
            wifi.sta.connect()
            wait_for_wifi()
        end
        )

    m:on("message", msg_callback or
    function(conn, topic, data)
        delivered = false
        for subs, cb in pairs(subscriptions) do
            path = MQTT_PATH .. "/" .. subs
            if topic == path then
                cb(conn, topic, data)
                delivered = true
            end
        end

        if not delivered then
            print("mtqq: received with no subs: " ..
                  topic .. ":" .. data)
        end

    end
    )

    wait_for_wifi()
    return m
end
