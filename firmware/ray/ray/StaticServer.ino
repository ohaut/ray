#include <ESP8266WebServer.h>
#include <FS.h>
#include "ray_global_defs.h"

#ifdef SPIFFLESS
  #include "../data.h"
#endif

#ifdef SPIFLESS
int32_t spiffs_hal_read(uint32_t addr, uint32_t size, uint8_t *dst);

class ConstReader {
  char _name[128];
  int _len;
  int _pos;
  PGM_P p;
public:
  ConstReader(const char *name, PGM_P data, int len)
  {
    p = data;
    strncpy(_name, name, 128);
    _len = len;
    _pos = 0;
    Serial.printf(">>0x%x\r\n", data);
  }

  int size() { return _len; }
  char* name() { return _name; } // use char* or String(x->name()) won't
                                        // work
  int available() { return _len - _pos; }
  int read(uint8_t* obuf, int len) {
    if (len>available()) len = available();
    //memcpy(obuf, p, len);
    //spiffs_hal_read(((uint32_t)p)-0x40200000, len, obuf);
    ESP.flashRead(((uint32_t)p)-0x40200000, (uint32_t*)obuf, len);
    //spiffs_hal_read(((uint32_t)p)-0x40200000, len, obuf);
    p += len;
    return len;
  }

};

bool spifless_exists(String s_name) {
  int i=0;
  const char* name = s_name.c_str();
  if (name[0]=='/') name++;
  while(spifless_names[i]) {
    if (strcmp(name, spifless_names[i]) == 0) {
      //Serial.printf("exists: [%s]\r\n", name);
      return true;
    }
    i++;
  }
  return false;
}

ConstReader *spifless_open(String s_name)
{
  int i=0;
  const char* name = s_name.c_str();
  if (name[0]=='/') name++;
  while(spifless_names[i]) {
    if (strcmp(name, spifless_names[i]) == 0) {
      return new ConstReader(name, spifless_datas[i], spifless_lengths[i]);
    }
    i++;
  }
  return NULL;
}
#endif /* SPIFLESS */


//format bytes
String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(ESP8266WebServer *server, String path){
  /*if (path.endsWith("config.tsv"))
  {
    server->send(403, "text/plain", "Unauthorized");
    return false;
  }*/
  if(path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
#ifndef SPIFLESS
  /* TODO(mangelajo): cleanup this ifdef mess to macros for file operations */
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
#else
  if (spifless_exists(pathWithGz) || spifless_exists(path)) {
    if (spifless_exists(pathWithGz))
#endif
      path += ".gz";

#ifndef SPIFLESS
    File file = SPIFFS.open(path, "r");
    size_t sent = server->streamFile(file, contentType);
    file.close();
#else
    ConstReader *file = spifless_open(path);

    size_t sent = server->streamFile(*file, contentType);
    delete file;
#endif
    return true;
  }
  return false;
}
