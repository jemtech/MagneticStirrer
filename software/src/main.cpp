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
#include <StirrerMenu.h>


StirrerConfig    stirrerConfig;
StirrerWebserver webServer;
StirrerDisplay   stirrerDisplay;
StirrerControl   stirrerControl;
StirrerEncoder   stirrerEncoder;
StirrerMenu      stirrerMenu;
StirrerWifi      stirrerWifi;
MqttHandler*     mqttHandler = nullptr;

const char* stirrerId = "0";  // hardcoded to stirrer 0, later use config
#define MQTT_PUBLISH_INTERVAL 1000  // ms
unsigned long lastMqttPublish = 0;

// Helper: log to Serial and display at once
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
    delay(100);  // keep it readable for a moment
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
    JsonDocument doc = stirrerConfig.toJson();
    char jsonResult[1024];
    serializeJson(doc, jsonResult);
    Serial.println(jsonResult);
    #endif
    log("Config OK", "");

    // WiFi
    log("Starting WiFi", "");
    stirrerWifi.setup(&(stirrerConfig.wifi), stirrerId);
    stirrerWifi.setupMdns(stirrerId);
    String ip = stirrerWifi.localIP().toString();
    log("IP:", ip.c_str());

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
    stirrerEncoder.setup();

    // MQTT
    if (stirrerConfig.mqtt.ip.length() > 0) {
        log("Starting MQTT", "");
        mqttHandler = new MqttHandler(stirrerId, &stirrerConfig);
        mqttHandler->setup();
        mqttHandler->publishStatus("online");
    } else {
        log("No MQTT server", "");
    }

    // Ready
    log("Ready!", "");
    delay(100);
    // From now on the menu shows the status page
    stirrerMenu.setup(&stirrerConfig, &stirrerControl, &stirrerEncoder, &stirrerDisplay);
}

void loop(void) {
    ArduinoOTA.handle();

    stirrerControl.update();

    stirrerMenu.update();

    if (mqttHandler != nullptr) {
        mqttHandler->loop();
        if (millis() - lastMqttPublish >= MQTT_PUBLISH_INTERVAL) {
            lastMqttPublish = millis();
            mqttHandler->publishSpeed((int)stirrerControl.getCurrentRpm());
            mqttHandler->publishTemperature(stirrerControl.isExtConnected()
                ? stirrerControl.getTempExt()
                : stirrerControl.getTempPlate());
            mqttHandler->publishPid(
                stirrerConfig.control.heaterEnabled,
                stirrerConfig.control.targetTemp,
                stirrerControl.getPidOutput(),
                stirrerConfig.control.motorEnabled,
                stirrerConfig.control.targetRpm,
                stirrerControl.getPidMotorOutput());
            mqttHandler->publishConfig();
        }
    }

    delay(1);
}
