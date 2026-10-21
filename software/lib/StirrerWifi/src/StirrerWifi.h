#pragma once
#include <StirrerConfig.h>

class StirrerWifi {
public:
    // stirrerId is set as DHCP hostname before WiFi.begin(),
    // so the router registers it as "hostname.lan".
    void setup(WifiConfig* wifiConfigPointer, const char* stirrerId);
    void setupMdns(const char* stirrerId);
    IPAddress localIP();
private:
    void fallBackAP(void);
};
