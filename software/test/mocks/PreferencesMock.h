#pragma once
#include <map>
#include <string>
#include <stdint.h>

// Mock for ESP32 Preferences - stores values in std::map
class Preferences {
public:
    void begin(const char* ns, bool readOnly = false) {
        _ns = ns;
    }
    void end() {}

    bool isKey(const char* key) {
        return _store.count(makeKey(key)) > 0;
    }

    void putString(const char* key, std::string val) {
        _store[makeKey(key)] = val;
    }
    std::string getString(const char* key, std::string def = "") {
        auto it = _store.find(makeKey(key));
        return it != _store.end() ? it->second : def;
    }

    void putFloat(const char* key, float val) {
        _store[makeKey(key)] = std::to_string(val);
    }
    float getFloat(const char* key, float def = 0.0f) {
        auto it = _store.find(makeKey(key));
        return it != _store.end() ? std::stof(it->second) : def;
    }

    void putDouble(const char* key, double val) {
        _store[makeKey(key)] = std::to_string(val);
    }
    double getDouble(const char* key, double def = 0.0) {
        auto it = _store.find(makeKey(key));
        return it != _store.end() ? std::stod(it->second) : def;
    }

    void putBool(const char* key, bool val) {
        _store[makeKey(key)] = val ? "1" : "0";
    }
    bool getBool(const char* key, bool def = false) {
        auto it = _store.find(makeKey(key));
        return it != _store.end() ? (it->second == "1") : def;
    }

    void putInt(const char* key, int val) {
        _store[makeKey(key)] = std::to_string(val);
    }
    int getInt(const char* key, int def = 0) {
        auto it = _store.find(makeKey(key));
        return it != _store.end() ? std::stoi(it->second) : def;
    }

    // Test helper: clear all stored values
    static void clearAll() { _store.clear(); }

private:
    std::string _ns;
    static std::map<std::string, std::string> _store;

    std::string makeKey(const char* key) {
        return _ns + ":" + key;
    }
};

std::map<std::string, std::string> Preferences::_store;
