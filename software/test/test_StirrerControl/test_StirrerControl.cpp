#include <unity.h>
#include <Arduino.h>
#include <Preferences.h>
#include <PID_v1.h>
#define private public  // tests set the private ISR state
#include "StirrerControl.h"
#undef private


// Helper: calculate ADC value for a given temperature
int tempToAdc(float tempC) {
    float tK   = tempC + 273.15f;
    float rNtc = 100000.0f * exp(3950.0f * (1.0f / tK - 1.0f / 298.15f));
    float v    = 3.3f * rNtc / (22000.0f + rNtc);
    return (int)(v / 3.3f * 4095.0f);
}

StirrerConfig cfg;

void setUp() {
    Preferences::resetAll();
    mock_analogRead_value  = 2048;
    mock_digitalRead_value = LOW;
    cfg.load();
    cfg.control.heaterEnabled = false;
    cfg.control.motorEnabled  = false;
    cfg.control.targetTemp    = 100.0f;
    cfg.control.tempPlateMax  = 300.0f;
    cfg.control.targetRpm     = 300.0f;
    cfg.control.maxRpm        = 1500.0f;
    cfg.control.useExtSensor  = false;
}
void tearDown() {}

// ── Sicherheitsabschaltung ────────────────────────────────────
void test_safety_shutdown_above_max() {
    mock_analogRead_value     = tempToAdc(310.0f);  // above 300°C
    cfg.control.heaterEnabled = true;
    cfg.control.tempPlateMax  = 300.0f;

    StirrerControl ctrl;
    ctrl.setup(&cfg);
    mock_millis_value = 1000;
    ctrl.update();

    TEST_ASSERT_FALSE(cfg.control.heaterEnabled);
}

void test_safety_no_shutdown_below_max() {
    mock_analogRead_value     = tempToAdc(150.0f);  // unter 300°C
    cfg.control.heaterEnabled = true;
    cfg.control.tempPlateMax  = 300.0f;

    StirrerControl ctrl;
    ctrl.setup(&cfg);
    mock_millis_value = 1000;
    ctrl.update();

    TEST_ASSERT_TRUE(cfg.control.heaterEnabled);
}

void test_safety_shutdown_on_sensor_error() {
    mock_analogRead_value     = 4095;   // NTC getrennt → -999°C
    cfg.control.heaterEnabled = true;

    StirrerControl ctrl;
    ctrl.setup(&cfg);
    mock_millis_value = 1000;
    ctrl.update();

    TEST_ASSERT_FALSE(cfg.control.heaterEnabled);
}

// ── Temperatursensor ─────────────────────────────────────────
void test_temp_plate_reading() {
    mock_analogRead_value = tempToAdc(80.0f);
    StirrerControl ctrl;
    ctrl.setup(&cfg);
    mock_millis_value = 1000;
    ctrl.update();
    TEST_ASSERT_FLOAT_WITHIN(5.0f, 80.0f, ctrl.getTempPlate());
}

void test_ext_sensor_not_connected() {
    cfg.control.useExtSensor = true;
    mock_analogRead_value    = 4095;  // offen

    StirrerControl ctrl;
    ctrl.setup(&cfg);
    mock_millis_value = 1000;
    ctrl.update();

    TEST_ASSERT_FALSE(ctrl.isExtConnected());
}

void test_ext_sensor_connected() {
    cfg.control.useExtSensor = true;
    mock_analogRead_value    = tempToAdc(60.0f);

    StirrerControl ctrl;
    ctrl.setup(&cfg);
    mock_millis_value = 1000;
    ctrl.update();

    TEST_ASSERT_TRUE(ctrl.isExtConnected());
}

// ── PID Output ───────────────────────────────────────────────
void test_pid_output_within_range() {
    mock_analogRead_value     = tempToAdc(50.0f);
    cfg.control.heaterEnabled = true;
    cfg.control.targetTemp    = 100.0f;

    StirrerControl ctrl;
    ctrl.setup(&cfg);
    mock_millis_value = 1000;
    ctrl.update();

    float out = ctrl.getPidOutput();
    TEST_ASSERT_GREATER_OR_EQUAL(0.0f,   out);
    TEST_ASSERT_LESS_OR_EQUAL   (100.0f, out);
}

void test_motor_pid_output_within_range() {
    cfg.control.motorEnabled = true;
    cfg.control.targetRpm    = 500.0f;
    // Hall-Sensor: kein Puls → 0 RPM
    mock_micros_value = 0;

    StirrerControl ctrl;
    ctrl.setup(&cfg);
    mock_millis_value = 1000;
    ctrl.update();

    float out = ctrl.getPidMotorOutput();
    TEST_ASSERT_GREATER_OR_EQUAL(0.0f,   out);
    TEST_ASSERT_LESS_OR_EQUAL   (100.0f, out);
}

// ── RPM Berechnung ───────────────────────────────────────────
void test_rpm_zero_when_no_pulse() {
    // Letzter Puls vor langer Zeit → 0 RPM
    StirrerControl::_lastPulseTime = 0;
    StirrerControl::_pulseInterval = 0;
    mock_micros_value = 5000000UL;  // 5 seconds later

    StirrerControl ctrl;
    ctrl.setup(&cfg);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, ctrl.getCurrentRpm());
}

void test_rpm_calculated_correctly() {
    // 1 Puls pro 100ms = 600 RPM
    StirrerControl::_lastPulseTime = 0;
    StirrerControl::_pulseInterval = 100000UL;  // 100ms in µs
    mock_micros_value              = 50000UL;   // innerhalb Timeout

    StirrerControl ctrl;
    ctrl.setup(&cfg);
    TEST_ASSERT_FLOAT_WITHIN(5.0f, 600.0f, ctrl.getCurrentRpm());
}

// ── Update Interval ──────────────────────────────────────────
void test_update_respects_interval() {
    mock_analogRead_value = tempToAdc(25.0f);
    StirrerControl ctrl;
    ctrl.setup(&cfg);

    // Erster Update bei t=0
    mock_millis_value = 0;
    ctrl.update();
    float temp1 = ctrl.getTempPlate();

    // Second update too early (before PID_INTERVAL_MS)
    mock_analogRead_value = tempToAdc(100.0f);
    mock_millis_value     = 100;  // nur 100ms – unter PID_INTERVAL_MS (500ms)
    ctrl.update();

    // Temperatur sollte noch nicht aktualisiert sein
    TEST_ASSERT_EQUAL_FLOAT(temp1, ctrl.getTempPlate());
}

// ── Main ─────────────────────────────────────────────────────
int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_safety_shutdown_above_max);
    RUN_TEST(test_safety_no_shutdown_below_max);
    RUN_TEST(test_safety_shutdown_on_sensor_error);
    RUN_TEST(test_temp_plate_reading);
    RUN_TEST(test_ext_sensor_not_connected);
    RUN_TEST(test_ext_sensor_connected);
    RUN_TEST(test_pid_output_within_range);
    RUN_TEST(test_motor_pid_output_within_range);
    RUN_TEST(test_rpm_zero_when_no_pulse);
    RUN_TEST(test_rpm_calculated_correctly);
    RUN_TEST(test_update_respects_interval);
    return UNITY_END();
}
