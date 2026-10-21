#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <functional>

struct WifiConfig {
    String ssid;
    String password;
};

struct MqttConfig {
    String ip;
    String user;
    String password;
};

struct PidConfig {
    double kp;
    double ki;
    double kd;
};

// Defaults are used until load() and written when nothing is stored yet.
struct StirrerControlConfig {
    float     targetTemp    = 25.0;
    float     tempPlateMax  = 300.0;
    float     targetRpm     = 300.0;
    float     maxRpm        = 1500.0;
    bool      useExtSensor  = false;
    bool      heaterEnabled = false;
    bool      motorEnabled  = false;
    PidConfig pidPlate      = { 5.0, 0.2, 2.0 };
    PidConfig pidExt        = { 3.0, 0.5, 1.0 };
    PidConfig pidMotor      = { 0.5, 0.1, 0.05 };
};

class StirrerConfig {
public:
    StirrerConfig();
    void load();
    void write();
    JsonDocument toJson();
    // Like toJson() but without passwords, safe to publish.
    JsonDocument toPublicJson();

    // Looks up a posted form field by name; returns false when it is missing.
    typedef std::function<bool(const char* name, String& value)> FormLookup;
    // Fills wifi + mqtt from the config form. Returns nullptr on success,
    // otherwise the name of the first missing field (config is then partly updated).
    const char* applyForm(const FormLookup& lookup);

    WifiConfig           wifi;
    MqttConfig           mqtt;
    StirrerControlConfig control;

private:
    Preferences preferences;

    void loadWifi();
    void loadMqtt();
    void loadControl();

    void writeWifi();
    void writeMqtt();
    void writeControl();
};
