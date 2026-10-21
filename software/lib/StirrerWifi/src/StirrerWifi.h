#pragma once
#include <StirrerConfig.h>

class StirrerWifi {
public:
    // stirrerId wird vor WiFi.begin() als DHCP-Hostname gesetzt,
    // damit der Router ihn als "hostname.lan" registriert.
    void setup(WifiConfig* wifiConfigPointer, const char* stirrerId);
    void setupMdns(const char* stirrerId);
    IPAddress localIP();
private:
    void fallBackAP(void);
};
