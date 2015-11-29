#ifndef __CONFIG_MAP
#define __CONFIG_MAP

class ConfigEntry;
class ConfigEntry {
  public:
  char *key;
  char *value;
  ConfigEntry* next;
  
  ConfigEntry(const char *key, const char* value) {
    this->key = strdup(key);
    this->value = strdup(value);
    this->next = NULL;
  }
  ~ConfigEntry() {
    free(key);
    free(value);
  }
  void update(const char *value) {
    free(this->value);
    this->value = strdup(value);
  }
};

class ConfigMap {
  ConfigEntry *entry_list;
  public:
  ConfigMap() {
    entry_list = NULL;
  }

  ConfigEntry* _find(const char *key) {
    ConfigEntry* p = entry_list;
    while (p) {
      if (strcmp(p->key, key)==0) return p;
      p = p->next;
    }
    return NULL;
    
  }
  
  void set(const char* key, const char* value) {
    ConfigEntry* p = _find(key);
    if (p) {
      p->update(value);
    } else {
      ConfigEntry *new_entry = new ConfigEntry(key, value);
      new_entry->next = entry_list;
      entry_list = new_entry;
    }
  }

  void set(const char* key, String value) {
    set(key, value.c_str());
  }
  char* operator[](const char* key) {
    ConfigEntry *p = _find(key);
    if (p) return p->value;
    return NULL;
  }
};

#endif

