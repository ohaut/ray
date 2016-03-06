#include <ESP8266WebServer.h>
#include "ConfigMap.h"

extern ConfigMap configData;

void setupDefaultConfigLamp() {
  configData.set("startup_val_l0", "0");
  configData.set("startup_val_l1", "0");
  configData.set("startup_val_l2", "0");
}

extern ESP8266WebServer* _server;

void handleConfigLamp() {
 String form =
    R"(<html>
      <head>
        <style>
          label { float:left; padding-left:2em; width: 12em; }
         </style>
      <body>
       <h1>Lamp mode configuration</h1>
       <form action="/config/lamp/" method="POST">
        <h2>Lamp startup settings (0-100 or empty to get it from net)</h2>
        <div><label>Channel 0</label><input name='startup_val_l0' value='$startup_val_l0'></div>
        <div><label>Channel 1</label><input name='startup_val_l1' value='$startup_val_l1'></div>
        <div><label>Channel 2</label><input name='startup_val_l2' value='$startup_val_l2'></div>
        <input type="submit" value="Save and Reboot">
       </form>
       </body>
      </html>)";

  configData.replaceVars(form);
  _server->send(200, "text/html", form);
}

void handleConfigLampPost() {
  handlePost(handleConfigLamp);
}
