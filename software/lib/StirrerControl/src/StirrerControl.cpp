#include "StirrerControl.h"

volatile unsigned long StirrerControl::_lastPulseTime = 0;
volatile unsigned long StirrerControl::_pulseInterval  = 0;

void IRAM_ATTR StirrerControl::isrHall() {
    unsigned long now = micros();
    _pulseInterval  = now - _lastPulseTime;
    _lastPulseTime  = now;
}

void StirrerControl::setup(StirrerConfig* config) {
    _config = config;

    _ntcPlate.setup();
    _ntcExt.setup();

    // Hall-Sensor
    pinMode(PIN_HALL, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_HALL), isrHall, FALLING);

    // Motor MOSFET - 20kHz for smooth operation
    ledcAttach(PIN_MOTOR,  20000, 8);

    // Heater SSR - 2Hz sufficient due to thermal inertia
    ledcAttach(PIN_HEATER, 2, 8);

    // Temperatur PID
    _pidTempSetpoint = _config->control.targetTemp;
    _pidTemp.SetMode(AUTOMATIC);
    _pidTemp.SetOutputLimits(0, 255);
    _pidTemp.SetSampleTime(PID_INTERVAL_MS);

    // Motor PID
    _pidMotorSetpoint = _config->control.targetRpm;
    _pidMotor.SetMode(AUTOMATIC);
    _pidMotor.SetOutputLimits(0, 255);
    _pidMotor.SetSampleTime(PID_MOTOR_INTERVAL_MS);

    _lastLoop = 0;
}

void StirrerControl::update() {
    unsigned long now = millis();  // FIX #3: unsigned long
    if (PID_INTERVAL_MS > now - _lastLoop) return;
    _lastLoop = now;

    // Temperaturen lesen
    _tempPlate    = _ntcPlate.readTemp();
    _extConnected = _ntcExt.isConnected() && _config->control.useExtSensor;
    if (_extConnected) _tempExt = _ntcExt.readTemp();

    // Sicherheitsabschaltung – immer Platten-NTC
    if (_tempPlate > _config->control.tempPlateMax || _tempPlate < -10.0) {
        _config->control.heaterEnabled = false;
        setHeater(0);
        #ifdef DEBUG
        Serial.printf("SAFETY: Heater off! Plate: %.1f°C\n", _tempPlate);
        #endif
    }

    // Temperatur PID
    _pidTempInput    = _extConnected ? _tempExt : _tempPlate;
    _pidTempSetpoint = _config->control.targetTemp;
    applyTempPidConfig();

    if (_config->control.heaterEnabled) {
        _pidTemp.Compute();
        setHeater((int)_pidTempOutput);
    } else {
        setHeater(0);
    }

    // Motor PID
    _pidMotorInput    = getRpm();
    _pidMotorSetpoint = _config->control.targetRpm;
    applyMotorPidConfig();

    if (_config->control.motorEnabled) {
        _pidMotor.Compute();
        setMotor((int)_pidMotorOutput);
    } else {
        setMotor(0);
    }

    #ifdef DEBUG
    Serial.printf("Plate: %.1f°C  Ext: %.1f°C (%s)  TempPID: %.0f  RPM: %.0f  MotorPID: %.0f\n",
        _tempPlate,
        _extConnected ? _tempExt : 0.0f,
        _extConnected ? "conn" : "disc",
        _pidTempOutput,
        getRpm(),
        _pidMotorOutput);
    #endif
}

float StirrerControl::getRpm() {
    if (micros() - _lastPulseTime > 2000000UL) return 0;
    noInterrupts();
    unsigned long interval = _pulseInterval;
    interrupts();
    if (interval == 0) return 0;
    return 60000000.0f / interval;
}

void StirrerControl::setHeater(int value) {
    ledcWrite(PIN_HEATER, constrain(value, 0, 255));
}

void StirrerControl::setMotor(int value) {
    if (value <= 0) {
        ledcWrite(PIN_MOTOR, 0);
        _motorWasOff   = true;
        _motorKickStart = 0;
        return;
    }

    // FIX #2: Nicht-blockierender Kick-Start – kein delay() mehr
    if (_motorWasOff) {
        // Kick-Start beginnen: Vollgas und Timestamp merken
        ledcWrite(PIN_MOTOR, 255);
        _motorKickStart = millis();
        _motorWasOff    = false;
        return;  // Erst beim nächsten update()-Aufruf weiter
    }

    // Kick-Start läuft noch? Warten ohne zu blockieren.
    if (_motorKickStart != 0) {
        if (millis() - _motorKickStart < MOTOR_KICKSTART_MS) return;
        // Kick-Start abgeschlossen: I-Term-Reset um Windup zu vermeiden
        _motorKickStart = 0;
        _pidMotor.SetMode(MANUAL);
        _pidMotorOutput = value;
        _pidMotor.SetMode(AUTOMATIC);
    }

    ledcWrite(PIN_MOTOR, constrain(value, 0, 255));
}

void StirrerControl::applyTempPidConfig() {
    PidConfig& cfg = _extConnected ? _config->control.pidExt
                                   : _config->control.pidPlate;
    _pidTemp.SetTunings(cfg.kp, cfg.ki, cfg.kd);
}

void StirrerControl::applyMotorPidConfig() {
    _pidMotor.SetTunings(
        _config->control.pidMotor.kp,
        _config->control.pidMotor.ki,
        _config->control.pidMotor.kd
    );
}
