// In-memory stand-in for the ESP32 NVS-backed Preferences class.
#ifndef MOCK_PREFERENCES_H
#define MOCK_PREFERENCES_H

#include <map>
#include <string>
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

    bool clear() {
        if (!open_ || readOnly_) return false;
        store()[ns_].clear();
        return true;
    }

    // test helper: wipe all namespaces, like erasing the NVS partition
    static void resetAll() { store().clear(); }

private:
    static std::map<std::string, std::map<std::string, std::string>>& store() {
        static std::map<std::string, std::map<std::string, std::string>> s;
        return s;
    }
    std::string ns_;
    bool readOnly_ = false;
    bool open_ = false;
};

#endif // MOCK_PREFERENCES_H
