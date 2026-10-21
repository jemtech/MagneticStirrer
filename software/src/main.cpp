#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <StirrerConfig.h>
#include <MqttHandler.h>
#include <StirrerWifi.h>
#include <StirrerWebserver.h>
#include <StirrerDisplay.h>
#include <StirrerControl.h>
#include <StirrerEncoder.h>


StirrerConfig    stirrerConfig;
StirrerWebserver webServer;
StirrerDisplay   stirrerDisplay;
StirrerControl   stirrerControl;
StirrerEncoder   stirrerEncoder;
StirrerWifi      stirrerWifi;
unsigned long lastLoop;
MqttHandler * mqttHandlerPointer = nullptr;
const char* stirrerId = "0"; // hardcoded to stirrer 0, later use config
StirrerWebserver webServer;
const int stirrerPin = 27;
long int stirrerVal;

// Hilfsfunktion: Serial + Display gleichzeitig
void log(const char* line1, const char* line2 = "") {
    #ifdef DEBUG
    Serial.print(line1);
    if (strlen(line2) > 0) {
        Serial.print(" ");
        Serial.print(line2);
    }
    Serial.println();
    #endif
    stirrerDisplay.write(line1, line2);
    delay(100);  // kurz lesbar lassen
}

void setup(void) {
    Serial.begin(115200);
    #ifdef DEBUG
    Serial.println("Boot");
    #endif

    // Display first - all subsequent logs via log()
    stirrerDisplay.setup();
    stirrerDisplay.write("Booting...", "");
    delay(100);

    // Config
    log("Loading config", "");
    stirrerConfig.load();
    #ifdef DEBUG
    JsonDocument doc;
    stirrerConfig.toJson(doc);
    char jsonResult[1024];
    serializeJson(doc, jsonResult);
    Serial.println(jsonResult);
    #endif
    log("Config OK", "");

    stirrerDisplay.setup();
    #ifdef DEBUG
    Serial.println("Display ready");
    #endif

    log("Starting WiFi", "");

    StirrerWifi stirrerWifi;
    stirrerWifi.setup(&(stirrerConfig.wifi));
    stirrerWifi.setupMdns(stirrerId);

    #ifdef DEBUG
    Serial.println("wifi setup");
    #endif

    stirrerDisplay.clear();
    String ipAddress = stirrerWifi.localIP().toString();
    stirrerDisplay.print(ipAddress.c_str());
    log("IP:", ipAddress.c_str());

    // OTA
    log("Starting OTA", "");
    ArduinoOTA.begin();
    log("OTA OK", "");

    // Webserver
    log("Starting Web", "");
    webServer.setup(&stirrerConfig, &stirrerControl);
    log("Webserver OK", "");

    // Control
    log("Starting Ctrl", "");
    stirrerControl.setup(&stirrerConfig);
    log("Control OK", "");

    // Encoder
    log("Starting Enc", "");
    stirrerEncoder.setup(&stirrerConfig, &stirrerDisplay);

    // Bereit
    log("Ready!", "");
    delay(100);
    // Encoder zeigt ab jetzt die Statusseite

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

void loop(void) {
    ArduinoOTA.handle();

    stirrerControl.update();

    stirrerEncoder.update();
    stirrerEncoder.updateSensorValues(
        stirrerControl.getTempPlate(),
        stirrerControl.getTempExt(),
        stirrerControl.isExtConnected(),
        stirrerControl.getCurrentRpm()
    );

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
