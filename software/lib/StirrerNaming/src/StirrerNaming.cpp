#include <StirrerNaming.h>
#include <stdio.h>
#include <string.h>

static const char* AP_SSID_PREFIX = "Stirrer-AP";

void buildMdnsHostName(char* out, size_t outSize, const char* stirrerId, const uint8_t mac[6]) {
    if (out == NULL || outSize == 0) return;
    if (stirrerId != NULL && strlen(stirrerId) > 0) {
        // %.110s keeps the name well inside the 128 byte host buffer used by the caller
        snprintf(out, outSize, "esp32stirrer_%.110s.lan", stirrerId);
    } else {
        snprintf(out, outSize, "esp32stirrer_%02X_%02X_%02X_%02X_%02X_%02X.lan",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
}

void buildApSsid(char* out, size_t outSize, const uint8_t mac[6]) {
    if (out == NULL || outSize == 0) return;
    snprintf(out, outSize, "%s-%02X:%02X:%02X:%02X:%02X:%02X", AP_SSID_PREFIX,
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
