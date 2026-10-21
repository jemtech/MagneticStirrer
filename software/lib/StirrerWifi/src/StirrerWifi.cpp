#include "StirrerWifi.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <StirrerNaming.h>

// Baut den Hostnamen aus stirrerId oder MAC-Adresse
static void buildHostname(char* buf, size_t len, const char* stirrerId) {
    if (strlen(stirrerId) > 0) {
        snprintf(buf, len, "esp32stirrer_%s", stirrerId);
    } else {
        byte mac[6];
        WiFi.macAddress(mac);
        snprintf(buf, len, "esp32stirrer_%02X_%02X_%02X_%02X_%02X_%02X",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
}

void StirrerWifi::setup(WifiConfig* wifiConfigPointer, const char* stirrerId) {
    delay(50);

    // FIX #8: DHCP-Hostname VOR WiFi.begin() setzen.
    // Der Router registriert ihn dann als "hostname.lan" in seiner DNS-Zone.
    char host[128];
    buildHostname(host, sizeof(host), stirrerId);
    WiFi.setHostname(host);

    if (wifiConfigPointer->ssid.length() < 1) {
        #ifdef DEBUG
        Serial.println("No SSID – starting fallback AP");
        #endif
        fallBackAP();
        return;
    }

    #ifdef DEBUG
    Serial.println("Connecting to " + wifiConfigPointer->ssid);
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
            return;
        }
    }
    #ifdef DEBUG
    Serial.println("\nWiFi connected – IP: " + WiFi.localIP().toString());
    Serial.println("DHCP hostname:  " + String(host) + "  →  DNS: " + String(host) + ".lan");
    #endif
}

IPAddress StirrerWifi::localIP() {
    return WiFi.localIP();
}

void StirrerWifi::setupMdns(const char* stirrerId) {
    // mDNS nur für Service-Discovery (z.B. dns-sd).
    // ".lan"-Auflösung läuft über Router-DNS, nicht über mDNS.
    // MDNS.begin() bekommt den Hostnamen OHNE TLD.
    char host[128];
    byte mac[6];
    WiFi.macAddress(mac);
    buildMdnsHostName(host, sizeof(host), stirrerId, mac);
    /*use mdns for host name resolution*/
    if (!MDNS.begin(host)) {
        #ifdef DEBUG
        Serial.println("Error setting up MDNS: " + String(host));
        #endif
        return;
    }
    #ifdef DEBUG
    Serial.println("mDNS: " + String(host) + ".local  (Service-Discovery)");
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
    Serial.println("Starting AP: " + String(apSsid));
    #endif
    WiFi.softAP(apSsid);
}
