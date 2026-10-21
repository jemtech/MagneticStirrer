#pragma once
#include <Arduino.h>
#include <PID_v1.h>
#include "NTCSensor.h"
#include "StirrerConfig.h"

// ── Pin-Definitionen ─────────────────────────────────────────
#define PIN_NTC_PLATE     34
#define PIN_NTC_EXT       35
#define PIN_HALL          13   // Platzhalter, noch nicht angeschlossen
#define PIN_HEATER        25   // SSR  - check solder joint
#define PIN_MOTOR         27   // MOSFET

// ── Timing ───────────────────────────────────────────────────
#define PID_INTERVAL_MS       500
#define PID_MOTOR_INTERVAL_MS 200
#define TEMP_PLATE_MAX    350.0

// ── PWM channels ────────────────────────────────────────────
#define PWM_CHANNEL_MOTOR   0
#define PWM_CHANNEL_HEATER  1

// ── Kick-Start Dauer ─────────────────────────────────────────
#define MOTOR_KICKSTART_MS  150

class StirrerControl {
public:
    StirrerControl()
        : _ntcPlate(PIN_NTC_PLATE, 22000.0, 100000.0, 25.0, 3950.0),
          _ntcExt  (PIN_NTC_EXT,   22000.0, 100000.0, 25.0, 3950.0),
          _pidTemp (&_pidTempInput,  &_pidTempOutput,  &_pidTempSetpoint,
                    2.0, 0.5, 1.0, DIRECT),
          _pidMotor(&_pidMotorInput, &_pidMotorOutput, &_pidMotorSetpoint,
                    0.5, 0.1, 0.05, DIRECT)
    {}

    void setup(StirrerConfig* config);
    void update();

    // Getters for webserver and encoder
    float getTempPlate()      { return _tempPlate; }
    float getTempExt()        { return _tempExt; }
    bool  isExtConnected()    { return _extConnected; }
    float getPidOutput()      { return (float)_pidTempOutput / 2.55f; }
    float getPidMotorOutput() { return (float)_pidMotorOutput / 2.55f; }
    float getCurrentRpm()     { return getRpm(); }
    float getTargetRpm()      { return _config->control.targetRpm; }

private:
    void setHeater(int value);
    void setMotor(int value);
    void applyTempPidConfig();
    void applyMotorPidConfig();
    float getRpm();

    NTCSensor _ntcPlate;
    NTCSensor _ntcExt;

    StirrerConfig* _config = nullptr;

    // Temperatur PID
    double _pidTempInput    = 0;
    double _pidTempOutput   = 0;
    double _pidTempSetpoint = 0;
    PID    _pidTemp;

    // Motor PID
    double _pidMotorInput    = 0;
    double _pidMotorOutput   = 0;
    double _pidMotorSetpoint = 0;
    PID    _pidMotor;

    // Messwerte
    float _tempPlate    = 0;
    float _tempExt      = 0;
    bool  _extConnected = false;

    // FIX #2: Kick-Start nicht-blockierend – Timestamp statt delay()
    bool          _motorWasOff      = true;
    unsigned long _motorKickStart   = 0;   // 0 = kein Kick-Start aktiv

    unsigned long _lastLoop = 0;   // FIX #3: unsigned long

    // Hall-Sensor ISR
    static volatile unsigned long _lastPulseTime;
    static volatile unsigned long _pulseInterval;
    static void IRAM_ATTR isrHall();
};
