
#include <StirrerWifi.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <StirrerNaming.h>

#define DEBUG

void StirrerWifi::setup(StirrerConfig::Wifi * wifiConfigPointer) {
    delay(50);

    if (wifiConfigPointer->ssid.length() < 1){
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
    byte mac[6];
    WiFi.macAddress(mac);
    buildMdnsHostName(host, sizeof(host), stirrerId, mac);
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

void StirrerWifi::fallBackAP(void){
    #ifdef DEBUG
    Serial.println();
    Serial.println("Start fall back AP");
    #endif

    byte mac[6];
    WiFi.macAddress(mac);
    char apSsid[64];
    buildApSsid(apSsid, sizeof(apSsid), mac);
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