#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoOTA.h>
#include <StirrerConfig.h>
/*#include <MqttHandler.h>*/
#include <StirrerWifi.h>
#include <StirrerWebserver.h>
#include <StirrerDisplay.h>

#define DEBUG


StirrerConfig stirrerConfig;
unsigned long lastLoop;
/*MqttHandler * mqttHandlerPointer;*/
StirrerWebserver webServer;
StirrerDisplay stirrerDisplay;

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

    stirrerDisplay.write("start Wifi");

    StirrerWifi stirrerWifi;
    stirrerWifi.setup(&(stirrerConfig.wifi));
    stirrerWifi.setupMdns("0"); // hardcode to stirrer 0 later use config

    #ifdef DEBUG
    Serial.println("wifi setup");
    #endif

    String ipAddress = stirrerWifi.localIP().toString();
    stirrerDisplay.write(ipAddress.c_str());

    ArduinoOTA.begin();

    #ifdef DEBUG
    Serial.println("ArduinoOTA online");
    #endif

    webServer.setup();

    #ifdef DEBUG
    Serial.println("webServer online");
    #endif

    lastLoop = 0;
/*
    if (strlen(sensorConfig.mqtt.ip) > 0) {
        mqttHandlerPointer = new MqttHandler(sensorConfig.sensor.id, &(sensorConfig.mqtt));

        mqttHandlerPointer->setup();
        mqttHandlerPointer->publishStatus("booted");
        mqttHandlerPointer->publishIp();

    #ifdef DEBUG
        Serial.println("MqttHandler online");
    } else {
        Serial.println("no mqtt server ip");
    #endif
    }
*/
  /* setup stirrer hardware*/
}

/* main loop */
void loop(void) {
    /* check for software update */
    ArduinoOTA.handle();

    // Check if 1000ms (1 second) has elapsed for periodic tasks
    if(millis() - lastLoop >= 1000) {
        lastLoop = millis();
        /*mqttHandlerPointer->publishStatus("sleep");*/
    }
    delay(1);
}