#pragma once
// Minimal ArduinoJson stub for native tests
// For complete JSON tests include ArduinoJson directly

#include <map>
#include <string>
#include <stdint.h>



struct JsonVariant {
    std::string value;
    bool isNull() const { return value.empty(); }
    float       as_float()  const { return value.empty() ? 0.0f : std::stof(value); }
    double      as_double() const { return value.empty() ? 0.0  : std::stod(value); }
    bool        as_bool()   const { return value == "1" || value == "true"; }
    int         as_int()    const { return value.empty() ? 0    : std::stoi(value); }
    template<typename T> T as() const;
    operator float()  const { return as_float(); }
    operator double() const { return as_double(); }
    operator bool()   const { return as_bool(); }
    operator int()    const { return as_int(); }
    template<typename T>
    void operator=(T v) { value = std::to_string(v); }
    void operator=(const char* v) { value = v; }
    void operator=(std::string v) { value = v; }
    struct JsonObject to(struct JsonObject) { return {}; }
};

template<> inline float   JsonVariant::as<float>()  const { return as_float(); }
template<> inline double  JsonVariant::as<double>() const { return as_double(); }
template<> inline bool    JsonVariant::as<bool>()   const { return as_bool(); }
template<> inline int     JsonVariant::as<int>()    const { return as_int(); }

struct JsonObject {
    std::map<std::string, JsonVariant> fields;
    JsonVariant& operator[](const char* key) { return fields[key]; }
    template<typename T>
    JsonObject to() { return {}; }
};

struct JsonDocument {
    std::map<std::string, JsonVariant> fields;
    JsonVariant& operator[](const char* key) { return fields[key]; }
    JsonObject   object(const char* key)     { return {}; }
    bool         isNull()                    { return fields.empty(); }
};

inline void serializeJson(JsonDocument&, ...) {}
inline void serializeJson(JsonDocument&, char*, size_t) {}
