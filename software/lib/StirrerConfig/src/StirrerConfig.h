#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>

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

struct StirrerControlConfig {
    float     targetTemp;
    float     tempPlateMax;
    float     targetRpm;
    float     maxRpm;
    bool      useExtSensor;
    bool      heaterEnabled;
    bool      motorEnabled;
    PidConfig pidPlate;
    PidConfig pidExt;
    PidConfig pidMotor;
};

class StirrerConfig {
public:
    StirrerConfig();
    void load();
    void write();
    void toJson(JsonDocument& doc);

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
