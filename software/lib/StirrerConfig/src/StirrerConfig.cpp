#include <StirrerConfig.h>

#define DEBUG
#define RW_MODE false  // read and write
#define RO_MODE true   // read only

StirrerConfig::StirrerConfig(){
    Preferences preferences;
}

void StirrerConfig::load(){
    #ifdef DEBUG
    Serial.println();
    Serial.println("Reading config");
    #endif
    loadWifi();
    loadMqtt();
    #ifdef DEBUG
    Serial.println();
    Serial.println("config loaded");
    #endif
}

void StirrerConfig::loadWifi(){
    preferences.begin("wifi", RO_MODE);
    bool ssidExsist = preferences.isKey("ssid");       // Test for the existence
    if (ssidExsist == false) {
        preferences.end();
        // not created jet set default and write
        #ifdef DEBUG
        Serial.println("loadWifi failed");
        Serial.println("set default");
        #endif
        wifi.ssid = "";
        wifi.password = "";
        writeWifi();
        preferences.begin("wifi", RO_MODE);
    }
    wifi.ssid = preferences.getString("ssid");
    wifi.password = preferences.getString("password");
    preferences.end();
}

const char* StirrerConfig::readPrefString(const char * key){
    String origString = preferences.getString(key);
    return origString.c_str();
}

void StirrerConfig::loadMqtt(){
    preferences.begin("mqtt", RO_MODE); // read only
    bool ipExsist = preferences.isKey("ip");       // Test for the existence
    if (ipExsist == false) {
        preferences.end();
        // not created jet set default and write
        #ifdef DEBUG
        Serial.println("loadMqtt failed");
        Serial.println("set default");
        #endif
        mqtt.ip = "";
        mqtt.user = "";
        mqtt.password = "";
        writeMqtt();
        preferences.begin("mqtt", RO_MODE); // read only
    }
    mqtt.ip = preferences.getString("ip");
    mqtt.user = preferences.getString("user");
    mqtt.password = preferences.getString("password");
    preferences.end();
}

void StirrerConfig::write(){
    #ifdef DEBUG
    Serial.println();
    Serial.println("Writing config");
    #endif
    writeWifi();
    writeMqtt();
    #ifdef DEBUG
    Serial.println();
    Serial.println("config written");
    #endif
}

void StirrerConfig::writeWifi(){
    preferences.begin("wifi", RW_MODE); // read and write
    #ifdef DEBUG
    Serial.println("write wifi");
    Serial.printf("write ssid:\"%s\"\n", wifi.ssid);
    #endif
    preferences.putString("ssid", wifi.ssid);
    #ifdef DEBUG
    Serial.printf("read ssid:\"%s\"\n", preferences.getString("ssid"));
    Serial.printf("write password:\"%s\"\n", wifi.password);
    #endif
    preferences.putString("password", wifi.password);
    #ifdef DEBUG
    Serial.printf("read password:\"%s\"\n", preferences.getString("password"));
    #endif
    preferences.end();
}

void StirrerConfig::writeMqtt(){
    preferences.begin("mqtt", RW_MODE); // read and write
    #ifdef DEBUG
    Serial.println("write mqtt");
    Serial.printf("write ip:\"%s\"\n", mqtt.ip);
    #endif
    preferences.putString("ip", mqtt.ip);
    preferences.putString("user", mqtt.user);
    preferences.putString("password", mqtt.password);
    preferences.end();
}

JsonDocument StirrerConfig::toJson(){
    JsonDocument doc;

    JsonObject wifiConfig =  doc["wifi"].to<JsonObject>();
    wifiConfig["ssid"] = wifi.ssid;
    wifiConfig["password"] = wifi.password;

    JsonObject mqttConfig =  doc["mqtt"].to<JsonObject>();
    mqttConfig["ip"] = mqtt.ip;
    mqttConfig["user"] = mqtt.user;
    mqttConfig["password"] = mqtt.password;

    return doc;
}
