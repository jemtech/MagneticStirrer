#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <StirrerConfig.h>
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

    // WiFi
    log("Starting WiFi", "");
    stirrerWifi.setup(&(stirrerConfig.wifi), "0");
    stirrerWifi.setupMdns("0");
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
    stirrerEncoder.setup(&stirrerConfig, &stirrerDisplay);

    // Bereit
    log("Ready!", "");
    delay(100);
    // Encoder zeigt ab jetzt die Statusseite
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

    delay(1);
}
