#include <StirrerConfig.h>

#define RW_MODE false
#define RO_MODE true

StirrerConfig::StirrerConfig() {}

void StirrerConfig::load() {
    #ifdef DEBUG
    Serial.println("\nReading config");
    #endif
    loadWifi();
    loadMqtt();
    loadControl();
    #ifdef DEBUG
    Serial.println("config loaded");
    #endif
}

void StirrerConfig::loadWifi() {
    preferences.begin("wifi", RO_MODE);
    bool exists = preferences.isKey("ssid");
    if (!exists) {
        preferences.end();
        #ifdef DEBUG
        Serial.println("loadWifi: no key, writing defaults");
        #endif
        wifi.ssid     = "";
        wifi.password = "";
        writeWifi();
        preferences.begin("wifi", RO_MODE);
    }
    wifi.ssid     = preferences.getString("ssid");
    wifi.password = preferences.getString("password");
    preferences.end();
}

void StirrerConfig::loadMqtt() {
    preferences.begin("mqtt", RO_MODE);
    bool exists = preferences.isKey("ip");
    if (!exists) {
        preferences.end();
        #ifdef DEBUG
        Serial.println("loadMqtt: no key, writing defaults");
        #endif
        mqtt.ip       = "";
        mqtt.user     = "";
        mqtt.password = "";
        writeMqtt();
        preferences.begin("mqtt", RO_MODE);
    }
    mqtt.ip       = preferences.getString("ip");
    mqtt.user     = preferences.getString("user");
    mqtt.password = preferences.getString("password");
    preferences.end();
}

void StirrerConfig::loadControl() {
    preferences.begin("control", RO_MODE);
    bool exists = preferences.isKey("targetTemp");
    if (!exists) {
        preferences.end();
        #ifdef DEBUG
        Serial.println("loadControl: no key, writing defaults");
        #endif
        control.targetTemp    = 25.0;
        control.tempPlateMax  = 300.0;
        control.targetRpm     = 300.0;
        control.maxRpm        = 1500.0;
        control.useExtSensor  = false;
        control.heaterEnabled = false;
        control.motorEnabled  = false;
        control.pidPlate  = { 5.0, 0.2, 2.0 };
        control.pidExt    = { 3.0, 0.5, 1.0 };
        control.pidMotor  = { 0.5, 0.1, 0.05 };
        writeControl();
        preferences.begin("control", RO_MODE);
    }
    control.targetTemp    = preferences.getFloat  ("targetTemp",   25.0);
    control.tempPlateMax  = preferences.getFloat  ("tempPlateMax", 300.0);
    control.targetRpm     = preferences.getFloat  ("targetRpm",    300.0);
    control.maxRpm        = preferences.getFloat  ("maxRpm",       1500.0);
    control.useExtSensor  = preferences.getBool   ("useExtSensor", false);
    control.heaterEnabled = preferences.getBool   ("heaterOn",     false);
    control.motorEnabled  = preferences.getBool   ("motorOn",      false);
    control.pidPlate.kp   = preferences.getDouble ("plt_kp",       5.0);
    control.pidPlate.ki   = preferences.getDouble ("plt_ki",       0.2);
    control.pidPlate.kd   = preferences.getDouble ("plt_kd",       2.0);
    control.pidExt.kp     = preferences.getDouble ("ext_kp",       3.0);
    control.pidExt.ki     = preferences.getDouble ("ext_ki",       0.5);
    control.pidExt.kd     = preferences.getDouble ("ext_kd",       1.0);
    control.pidMotor.kp   = preferences.getDouble ("mot_kp",       0.5);
    control.pidMotor.ki   = preferences.getDouble ("mot_ki",       0.1);
    control.pidMotor.kd   = preferences.getDouble ("mot_kd",       0.05);
    preferences.end();
}

void StirrerConfig::write() {
    #ifdef DEBUG
    Serial.println("\nWriting config");
    #endif
    writeWifi();
    writeMqtt();
    writeControl();
    #ifdef DEBUG
    Serial.println("config written");
    #endif
}

void StirrerConfig::writeWifi() {
    preferences.begin("wifi", RW_MODE);
    preferences.putString("ssid",     wifi.ssid);
    preferences.putString("password", wifi.password);
    preferences.end();
}

void StirrerConfig::writeMqtt() {
    preferences.begin("mqtt", RW_MODE);
    preferences.putString("ip",       mqtt.ip);
    preferences.putString("user",     mqtt.user);
    preferences.putString("password", mqtt.password);
    preferences.end();
}

void StirrerConfig::writeControl() {
    preferences.begin("control", RW_MODE);
    preferences.putFloat  ("targetTemp",   control.targetTemp);
    preferences.putFloat  ("tempPlateMax", control.tempPlateMax);
    preferences.putFloat  ("targetRpm",    control.targetRpm);
    preferences.putFloat  ("maxRpm",       control.maxRpm);
    preferences.putBool   ("useExtSensor", control.useExtSensor);
    preferences.putBool   ("heaterOn",     control.heaterEnabled);
    preferences.putBool   ("motorOn",      control.motorEnabled);
    preferences.putDouble ("plt_kp",       control.pidPlate.kp);
    preferences.putDouble ("plt_ki",       control.pidPlate.ki);
    preferences.putDouble ("plt_kd",       control.pidPlate.kd);
    preferences.putDouble ("ext_kp",       control.pidExt.kp);
    preferences.putDouble ("ext_ki",       control.pidExt.ki);
    preferences.putDouble ("ext_kd",       control.pidExt.kd);
    preferences.putDouble ("mot_kp",       control.pidMotor.kp);
    preferences.putDouble ("mot_ki",       control.pidMotor.ki);
    preferences.putDouble ("mot_kd",       control.pidMotor.kd);
    preferences.end();
}

void StirrerConfig::toJson(JsonDocument& doc) {

    JsonObject wifiObj = doc["wifi"].to<JsonObject>();
    wifiObj["ssid"]     = wifi.ssid;
    wifiObj["password"] = wifi.password;

    JsonObject mqttObj = doc["mqtt"].to<JsonObject>();
    mqttObj["ip"]       = mqtt.ip;
    mqttObj["user"]     = mqtt.user;
    mqttObj["password"] = mqtt.password;

    JsonObject ctrl = doc["control"].to<JsonObject>();
    ctrl["targetTemp"]    = control.targetTemp;
    ctrl["tempPlateMax"]  = control.tempPlateMax;
    ctrl["targetRpm"]     = control.targetRpm;
    ctrl["maxRpm"]        = control.maxRpm;
    ctrl["useExtSensor"]  = control.useExtSensor;
    ctrl["heaterEnabled"] = control.heaterEnabled;
    ctrl["motorEnabled"]  = control.motorEnabled;

    JsonObject pidPlate = ctrl["pidPlate"].to<JsonObject>();
    pidPlate["kp"] = control.pidPlate.kp;
    pidPlate["ki"] = control.pidPlate.ki;
    pidPlate["kd"] = control.pidPlate.kd;

    JsonObject pidExt = ctrl["pidExt"].to<JsonObject>();
    pidExt["kp"] = control.pidExt.kp;
    pidExt["ki"] = control.pidExt.ki;
    pidExt["kd"] = control.pidExt.kd;

    JsonObject pidMotor = ctrl["pidMotor"].to<JsonObject>();
    pidMotor["kp"] = control.pidMotor.kp;
    pidMotor["ki"] = control.pidMotor.ki;
    pidMotor["kd"] = control.pidMotor.kd;

}
