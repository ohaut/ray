#include <Ticker.h>
#include <OHAUTlib.h>
#include "LEDDimmers.h"
#include "version.h"


#define DEVICE_TYPE "3CHANLED"

OHAUTservice ohaut;
LEDDimmers dimmers;

#if defined(ESP8266)
int led_pin = 13;
#elif defined(ESP32)
int led_pin = NO_LED_PIN;
#endif

char lamp_name1[128];
char lamp_name2[128];
char lamp_name3[128];
char lamp_nameall[128];
float boot_values[3];
float proportional_values[3];

typedef enum {
    STARTUP_OFF = 0,
    STARTUP_ON,
    STARTUP_P_SWITCH,
    STARTUP_ON_PB,
    STARTUP_OFF_PB,
} startup_mode_t;

startup_mode_t startup_mode = STARTUP_OFF;

void setup(void){

   /* start the serial port and switch on the PCB led */
    Serial.begin(115200);

    ohaut.set_led_pin(led_pin);

    ohaut.on_config_defaults = [](ConfigMap *config) {
        config->set("mode", "lamp");

        config->set("startup_val_l0", "100");
        config->set("startup_val_l1", "0");
        config->set("startup_val_l2", "0");

        config->set("pub_l0_bool", "false");
        config->set("pub_l1_bool", "false");
        config->set("pub_l2_bool", "false");
        config->set("pub_all_bool", "true");
        config->set("startup_mode", "1");

        config->set("all_mode", "0"); /* default mode proportional */

    };

    ohaut.on_config_loaded = [](ConfigMap *configData) {
        for (int led=0;led<3; led++)
            boot_values[led] = getDimmerStartupVal(configData, led);

        /* switch on leds */
        dimmers.setup(boot_values, atoi((*configData)["all_mode"]));

        // Add virtual devices
        sprintf(lamp_name1, "%s_l0", ohaut.get_host_id());
        sprintf(lamp_name2, "%s_l1", ohaut.get_host_id());
        sprintf(lamp_name3, "%s_l2", ohaut.get_host_id());
        sprintf(lamp_nameall, "%s_all", ohaut.get_host_id());

        if (configData->isTrue("pub_l0_bool")) ohaut.fauxmo->addDevice(lamp_name1);
        if (configData->isTrue("pub_l1_bool")) ohaut.fauxmo->addDevice(lamp_name2);
        if (configData->isTrue("pub_l2_bool")) ohaut.fauxmo->addDevice(lamp_name3);
        if (configData->isTrue("pub_all_bool")) ohaut.fauxmo->addDevice(lamp_nameall);

        startup_mode = getStartupMode(configData);
    };

    ohaut.on_http_server_ready = &setupHTTPApi;

    ohaut.on_ota_start = [](){
        dimmers.halt();
    };

    ohaut.on_ota_error = [](ota_error_t error) {
        dimmers.restart();
    };

    ohaut.on_ota_end =  [](){
       #ifdef ESP8266
        for (int i=0;i<30;i++){
            analogWrite(led_pin,(i*100) % 1001);
            delay(50);
        }
       #endif
    };

    ohaut.setup(DEVICE_TYPE, VERSION, "ray");

    ohaut.fauxmo->onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {

        Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\r\n", device_id, device_name, state ? "ON" : "OFF", value);

        // Checking for device_id is simpler if you are certain about the order they are loaded and it does not change.
        // Otherwise comparing the device_name is safer.
        float valf;

        valf = state ? ((float)value/254.0) : 0.0;
        if (valf>1.0) {
            valf = 1.0;
        }

        if (strcmp(device_name, lamp_name1)==0) {
            dimmers.setDimmer(0, valf);
        } else if (strcmp(device_name, lamp_name2)==0) {
            dimmers.setDimmer(1, valf);
        } else if (strcmp(device_name, lamp_name3)==0) {
            dimmers.setDimmer(2, valf);
        } else if (strcmp(device_name, lamp_nameall)==0) {
            dimmers.setAll(valf);
        }
    });
}

void loop(void){
    ohaut.handle();
    delay(50);
}

float getDimmerStartupVal(ConfigMap *configData, int dimmer) {
    char key[16];
    const char *val;
    sprintf(key, "startup_val_l%d", dimmer);
    val = (*configData)[key];

    if (val && strlen(val)) return atoi(val)/100.0;
    else                    return 1.0;
}

startup_mode_t getStartupMode(ConfigMap *configData) {
    const char *val = (*configData)["startup_mode"];
    if (val && strlen(val)) return (startup_mode_t)atoi(val);
    else                    return STARTUP_OFF;
}
