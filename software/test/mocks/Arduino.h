#pragma once

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <string>

// Arduino mock for the PlatformIO native tests.

typedef uint8_t byte;
typedef bool    boolean;

#define INPUT        0
#define OUTPUT       1
#define INPUT_PULLUP 2
#define HIGH         1
#define LOW          0
#define CHANGE       1
#define FALLING      2
#define RISING       3
#define IRAM_ATTR

#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

// Values returned by the hardware stubs, set by the tests.
inline int           mock_analogRead_value  = 0;
inline int           mock_digitalRead_value = 0;
inline unsigned long mock_millis_value      = 0;
inline unsigned long mock_micros_value      = 0;

inline int           analogRead(int)        { return mock_analogRead_value; }
inline int           digitalRead(int)       { return mock_digitalRead_value; }
inline void          pinMode(int, int)      {}
inline void          digitalWrite(int, int) {}
inline void          delay(unsigned long)   {}
inline unsigned long millis()               { return mock_millis_value; }
inline unsigned long micros()               { return mock_micros_value; }
inline void          noInterrupts()         {}
inline void          interrupts()           {}
inline int           digitalPinToInterrupt(int pin) { return pin; }
inline void          attachInterrupt(int, void (*)(), int) {}
inline bool          ledcAttach(int, unsigned long, int)   { return true; }
inline bool          ledcWrite(int, int)                    { return true; }

class String {
public:
    String() {}
    String(const char* s) : s_(s ? s : "") {}
    String(const std::string& s) : s_(s) {}

    String& operator=(const char* s) { s_ = s ? s : ""; return *this; }

    const char* c_str() const { return s_.c_str(); }
    unsigned int length() const { return static_cast<unsigned int>(s_.length()); }
    bool operator==(const String& o) const { return s_ == o.s_; }
    bool operator==(const char* o) const { return s_ == o; }

    // used by ArduinoJson's Arduino String writer
    unsigned char concat(char c) { s_ += c; return 1; }
    unsigned char concat(const char* s) { s_ += s; return 1; }
    unsigned char concat(const char* s, unsigned int n) { s_.append(s, n); return 1; }
    unsigned char reserve(unsigned int n) { s_.reserve(n); return 1; }

private:
    std::string s_;
};

// Serial output is discarded so test runs stay readable.
class MockSerial {
public:
    void begin(unsigned long) {}
    size_t print(const char*) { return 0; }
    size_t println(const char* = "") { return 0; }
    int printf(const char*, ...) __attribute__((format(printf, 2, 3))) { return 0; }
};
inline MockSerial Serial;
