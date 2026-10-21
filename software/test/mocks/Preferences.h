#pragma once

#include <map>
#include <string>
#include <cstdio>
#include <Arduino.h>

class Preferences {
public:
    bool begin(const char* name, bool readOnly = false) {
        ns_ = name;
        readOnly_ = readOnly;
        open_ = true;
        return true;
    }
    void end() { open_ = false; }

    bool isKey(const char* key) {
        if (!open_) return false;
        auto& space = store()[ns_];
        return space.find(key) != space.end();
    }

    String getString(const char* key, const String defaultValue = String()) {
        if (!open_) return defaultValue;
        auto& space = store()[ns_];
        auto it = space.find(key);
        return it == space.end() ? defaultValue : String(it->second);
    }

    size_t putString(const char* key, const String value) {
        if (!open_ || readOnly_) return 0;
        store()[ns_][key] = value.c_str();
        return value.length();
    }

    size_t putFloat(const char* key, float value)   { return putNumber(key, value); }
    size_t putDouble(const char* key, double value) { return putNumber(key, value); }
    size_t putBool(const char* key, bool value)     { return putNumber(key, value ? 1 : 0); }

    float getFloat(const char* key, float defaultValue = 0)    { return (float)getNumber(key, defaultValue); }
    double getDouble(const char* key, double defaultValue = 0) { return getNumber(key, defaultValue); }
    bool getBool(const char* key, bool defaultValue = false)   { return getNumber(key, defaultValue ? 1 : 0) != 0; }

    bool clear() {
        if (!open_ || readOnly_) return false;
        store()[ns_].clear();
        return true;
    }

    // test helper: wipe all namespaces, like erasing the NVS partition
    static void resetAll() { store().clear(); }

private:
    size_t putNumber(const char* key, double value) {
        if (!open_ || readOnly_) return 0;
        char buf[32];
        snprintf(buf, sizeof(buf), "%.17g", value);
        store()[ns_][key] = buf;
        return sizeof(double);
    }
    double getNumber(const char* key, double defaultValue) {
        if (!open_) return defaultValue;
        auto& space = store()[ns_];
        auto it = space.find(key);
        return it == space.end() ? defaultValue : std::stod(it->second);
    }

    static std::map<std::string, std::map<std::string, std::string>>& store() {
        static std::map<std::string, std::map<std::string, std::string>> s;
        return s;
    }
    std::string ns_;
    bool readOnly_ = false;
    bool open_ = false;
};
