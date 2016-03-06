#include "ray_global_defs.h"

void handleSetLed() {

    int ch , val;
    char* str_ch = strdup(server.arg("ch").c_str());
    char* str_val = strdup(server.arg("val").c_str());

    if (!strlen(str_val)) { /* we asume no ch means all channels */
      server.send(422, "text/html", "val parameter missing on URL");
      goto _exit;
    }
    ch = atoi(str_ch);
    val = atoi(str_val);

    if (ch<0 || ch>=3) {
      server.send(422, "text/html", "ch out of range (0..2)");
      goto _exit;
    }

    if (val<0 || val>100) {
      server.send(422, "text/html", "val out of range (0..100)");
      goto _exit;
    }

    if (strlen(str_ch))
      setDimmerAndPublish(ch, val);
    else {
      for (ch=0; ch<N_DIMMERS; ch++)
        setDimmerAndPublish(ch, val);
    }
    server.send(200, "text/html", "channel set correctly");
 _exit:
    free (str_ch);
    free (str_val);
}

void setupHTTPApi(ESP8266WebServer *server) {
  server->on("/setLed", HTTP_GET,  handleSetLed);
}
