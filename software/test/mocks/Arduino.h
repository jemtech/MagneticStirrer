// Minimal host-side stand-in for the Arduino core, just enough for native unit tests.
#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

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

#endif // MOCK_ARDUINO_H
