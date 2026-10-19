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
                String ssid;
                String password;
            };
        Wifi wifi;
        struct Mqtt {
                String ip;
                String user;
                String password;
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