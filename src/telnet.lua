local telnet_started = false
BANNER = "NodeMcu telnet server"

function start_telnet_server(port)
    telnet_port = port or 2323
    if (telnet_started) then
        return
    end

    s=net.createServer(net.TCP,180)

    s:listen(telnet_port,function(connection)
       function s_output(str)
          if(connection~=nil)
             then connection:send(str)
          end
       end
       node.output(s_output, 0)
       connection:on("receive",function(c,l)
          node.input(l)
       end)
       connection:on("disconnection",function(c)
          node.output(nil)
       end)
       print(BANNER)
    end)
    telnet_started = true
end

start_telnet_server()
