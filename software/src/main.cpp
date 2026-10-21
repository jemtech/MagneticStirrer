#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoOTA.h>
#include <StirrerConfig.h>
#include <MqttHandler.h>
#include <StirrerWifi.h>
#include <StirrerWebserver.h>
#include <StirrerDisplay.h>

#define DEBUG


StirrerConfig stirrerConfig;
unsigned long lastLoop;
MqttHandler * mqttHandlerPointer = nullptr;
const char* stirrerId = "0"; // hardcoded to stirrer 0, later use config
StirrerWebserver webServer;
StirrerDisplay stirrerDisplay;
const int stirrerPin = 27;
long int stirrerVal;

/* setup function */
void setup(void) {
    Serial.begin(115200);
    #ifdef DEBUG
    Serial.println("Boot");
    #endif

    stirrerConfig.load();

    #ifdef DEBUG
    Serial.println("config loaded");
    Serial.print("config: ");
    JsonDocument doc = stirrerConfig.toJson();
    char jsonResult[1024];
    serializeJson(doc, jsonResult);
    Serial.println(jsonResult);
    #endif

    stirrerDisplay.setup();
    #ifdef DEBUG
    Serial.println("Display ready");
    #endif

    stirrerDisplay.clear();
    stirrerDisplay.print("start Wifi");

    StirrerWifi stirrerWifi;
    stirrerWifi.setup(&(stirrerConfig.wifi));
    stirrerWifi.setupMdns(stirrerId);

    #ifdef DEBUG
    Serial.println("wifi setup");
    #endif

    stirrerDisplay.clear();
    String ipAddress = stirrerWifi.localIP().toString();
    stirrerDisplay.print(ipAddress.c_str());

    ArduinoOTA.begin();

    #ifdef DEBUG
    Serial.println("ArduinoOTA online");
    #endif

    webServer.setup();

    #ifdef DEBUG
    Serial.println("webServer online");
    #endif

    lastLoop = 0;
    if (stirrerConfig.mqtt.ip.length() > 0) {
        mqttHandlerPointer = new MqttHandler(stirrerId, &stirrerConfig);

        mqttHandlerPointer->setup();
        mqttHandlerPointer->publishStatus("booted");

    #ifdef DEBUG
        Serial.println("MqttHandler online");
    } else {
        Serial.println("no mqtt server ip");
    #endif
    }
  /* setup stirrer hardware*/
  
    pinMode(stirrerPin, OUTPUT);
    stirrerVal = 0;
}


void stirrerOn(int value){
    if(value == stirrerVal) return;
    stirrerVal = value;
    analogWrite(stirrerPin, value);
    if(mqttHandlerPointer != nullptr) {
        mqttHandlerPointer->publishSpeed(value);
    }
}

void stirrerOff(){
    stirrerOn(0);
}

/* main loop */
void loop(void) {
    /* check for software update */
    ArduinoOTA.handle();

    if(2000 < millis() && 4000 > millis()) {
        stirrerOn(127);
    } else if(4000 < millis() && 6000 > millis()) {
        stirrerOn(180);
    } else if(6000 < millis() && 8000 > millis()) {
        stirrerOn(220);
    } else if(8000 < millis() && 10000 > millis()) {
        stirrerOn(255);
    } else {
        stirrerOff();
    }
    
    if(mqttHandlerPointer != nullptr) {
        mqttHandlerPointer->loop();
    }

    if(millis() - lastLoop >= 1000) {
        lastLoop = millis();
        if(mqttHandlerPointer != nullptr) {
            mqttHandlerPointer->publishStatus("sleep");
        }
    }
    delay(1);
}