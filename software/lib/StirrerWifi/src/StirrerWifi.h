#ifndef STIRRERWIFI_H // include guard
#define STIRRERWIFI_H

#include <StirrerConfig.h>

class StirrerWifi{
    public:
        void setup(StirrerConfig::Wifi * wifiConfigPointer);
        void setupMdns(const char* stirrerId);
        IPAddress localIP();
    private:
        void fallBackAP(void);
};

#endif //STIRRERWIFI_H