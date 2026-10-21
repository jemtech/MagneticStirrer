#ifndef StirrerNaming_H // include guard
#define StirrerNaming_H

#include <stddef.h>
#include <stdint.h>

// Hardware independent helpers for network names, so they can be unit tested on the host.

// "esp32stirrer_<id>" (id truncated to fit), or the MAC based name when stirrerId is empty.
// Without TLD: used as DHCP hostname (router serves it as "<host>.lan") and for MDNS.begin().
void buildHostName(char* out, size_t outSize, const char* stirrerId, const uint8_t mac[6]);

// "Stirrer-AP-AA:BB:CC:DD:EE:FF" (28 chars, within the 32 byte SSID limit)
void buildApSsid(char* out, size_t outSize, const uint8_t mac[6]);

#endif // StirrerNaming_H
