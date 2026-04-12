#ifndef StirrerConfig_H // include guard
#define StirrerConfig_H

#include <Preferences.h>
#include <ArduinoJson.h>


class StirrerConfig{
    public:
        StirrerConfig();
        void load();
        void write();
        JsonDocument toJson();
        struct Wifi {
                const char* ssid;
                const char* password;
            };
        Wifi wifi;
        struct Mqtt {
                const char* ip;
                const char* user;
                const char* password;
            };
        Mqtt mqtt;
    private:
        Preferences preferences;
        void loadWifi();
        void writeWifi();
        void loadMqtt();
        void writeMqtt();
        const char* readPrefString(const char * key);
};
#endif // StirrerConfig_H