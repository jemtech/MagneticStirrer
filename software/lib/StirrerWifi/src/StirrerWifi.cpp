
#include <StirrerWifi.h>
#include <WiFi.h>
#include <ESPmDNS.h>

#define DEBUG

void StirrerWifi::setup(StirrerConfig::Wifi * wifiConfigPointer) {
    delay(50);

    if (strlen(wifiConfigPointer->ssid) < 1){
        #ifdef DEBUG
        Serial.println();
        Serial.println("No SSID using Fallback ");
        #endif
        fallBackAP();
        return;
    }

    // We start by connecting to a WiFi network
    #ifdef DEBUG
    Serial.println();
    Serial.println("Connecting to ");
    Serial.println(wifiConfigPointer->ssid);
    #endif

    WiFi.begin(wifiConfigPointer->ssid.c_str(), wifiConfigPointer->password.c_str());
    int wifiTries = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        wifiTries++;
        #ifdef DEBUG
        Serial.print(".");
        #endif
        if(wifiTries >= 10){
            fallBackAP();
            //old logic
            //reboots the chip
            //esp_sleep_enable_timer_wakeup(100); //micro seconds
            //esp_deep_sleep_start();
            return;
        }
    }
    #ifdef DEBUG
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    #endif
}

IPAddress StirrerWifi::localIP(){
    return WiFi.localIP();
}

void StirrerWifi::setupMdns(const char* stirrerId){
    char host[128];
    if (strlen(stirrerId) > 0) {
        // Ensure stirrerId doesn't overflow buffer (128 - "esp32stirrer_.lan\0" = ~110 chars max)
        if (strlen(stirrerId) > 110) {
            #ifdef DEBUG
            Serial.println("stirrerId too long, truncating");
            #endif
            snprintf(host, sizeof(host), "esp32stirrer_%.110s.lan", stirrerId);
        } else {
            snprintf(host, sizeof(host), "esp32stirrer_%s.lan", stirrerId);
        }
    } else {
        #ifdef DEBUG
        Serial.println("no stirrer id use mac");
        #endif
        byte mac[6];
        WiFi.macAddress(mac);
        snprintf(host, sizeof(host), "esp32stirrer_%02X_%02X_%02X_%02X_%02X_%02X.lan",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    }
    /*use mdns for host name resolution*/
    if (!MDNS.begin(host)) {
        #ifdef DEBUG
        Serial.println("Error setting up MDNS responder!");
        Serial.println(host);
        #endif
        return;
    }
    #ifdef DEBUG
    Serial.println("mDNS responder started");
    Serial.println(host);
    #endif
}

const char* AP_SSID_PREFIX = "ESP32-Stirrer-AP";

void StirrerWifi::fallBackAP(void){
    #ifdef DEBUG
    Serial.println();
    Serial.println("Start fall back AP");
    #endif

    byte mac[6];
    WiFi.macAddress(mac);
    char apSsid[18 + strlen(AP_SSID_PREFIX)];
    sprintf(apSsid,"%s-%02X:%02X:%02X:%02X:%02X:%02X", AP_SSID_PREFIX, mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    #ifdef DEBUG
    Serial.println();
    Serial.print("ssid: ");
    Serial.print(apSsid);
    Serial.println();
    #endif
    
    WiFi.softAP(apSsid);

    #ifdef DEBUG
    Serial.println("softAP started");
    #endif
}