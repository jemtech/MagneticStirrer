#ifndef StirrerConfig_H // include guard
#define StirrerConfig_H

#include <Preferences.h>
#include <ArduinoJson.h>
#include <functional>


class StirrerConfig{
    public:
        StirrerConfig();
        void load();
        void write();
        JsonDocument toJson();
        // Like toJson() but without passwords, safe to publish.
        JsonDocument toPublicJson();
        // Looks up a posted form field by name; returns false when it is missing.
        typedef std::function<bool(const char* name, String& value)> FormLookup;
        // Fills the config from the config form. Returns nullptr on success,
        // otherwise the name of the first missing field (config is then partly updated).
        const char* applyForm(const FormLookup& lookup);
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
};
#endif // StirrerConfig_H