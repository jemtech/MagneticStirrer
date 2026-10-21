#pragma once
// Arduino mock for PlatformIO native tests
// Replaces hardware dependencies with testable stubs
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// ── Typen ────────────────────────────────────────────────────
typedef uint8_t  byte;
typedef bool     boolean;

// ── Konstanten ───────────────────────────────────────────────
#define INPUT       0
#define OUTPUT      1
#define INPUT_PULLUP 2
#define HIGH        1
#define LOW         0
#define DIRECT      0
#define AUTOMATIC   1
#define MANUAL      0
#define CHANGE      1
#define FALLING     2
#define RISING      3

// ── Mock ADC value (overridable by tests) ───────────────────
extern int mock_analogRead_value;
extern int mock_digitalRead_value;
extern unsigned long mock_millis_value;
extern unsigned long mock_micros_value;

inline int      analogRead(int)        { return mock_analogRead_value; }
inline int      digitalRead(int)       { return mock_digitalRead_value; }
inline void     pinMode(int, int)      {}
inline void     digitalWrite(int, int) {}
inline void     delay(int)             {}
inline void     noInterrupts()         {}
inline void     interrupts()           {}
inline void     attachInterrupt(int, void(*)(), int) {}
inline int      digitalPinToInterrupt(int pin) { return pin; }
inline unsigned long millis()  { return mock_millis_value; }
inline unsigned long micros()  { return mock_micros_value; }
inline void     ledcSetup(int, int, int)    {}
inline void     ledcAttachPin(int, int)     {}
inline void     ledcWrite(int, int)         {}
inline int      ledcRead(int)               { return 0; }
inline int      constrain(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}
inline float constrain(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// ── String Klasse ────────────────────────────────────────────
#include <string>
typedef std::string String;

// ── Serial Mock ──────────────────────────────────────────────
struct SerialMock {
    void begin(int) {}
    void println(const char* s = "") { printf("%s\n", s); }
    void print(const char* s)        { printf("%s", s); }
    void printf(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }
} Serial;

// ── IRAM_ATTR ────────────────────────────────────────────────
#define IRAM_ATTR
