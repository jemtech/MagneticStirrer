#include <unity.h>
#include <Arduino.h>
#include <Preferences.h>
#include <StirrerDisplay.h>
#define private public  // tests set the private ISR and sensor state
#include "StirrerEncoder.h"
#include "StirrerControl.h"
#include "StirrerMenu.h"
#undef private

StirrerConfig  cfg;
StirrerDisplay display;
StirrerControl control;
StirrerEncoder enc;
StirrerMenu    menu;

void setUp() {
    Preferences::resetAll();
    mock_digitalRead_value = HIGH;  // Button not pressed
    mock_millis_value      = 0;
    mock_micros_value      = 0;
    cfg.load();
    cfg.control.targetTemp    = 100.0f;
    cfg.control.targetRpm     = 300.0f;
    cfg.control.tempPlateMax  = 300.0f;
    cfg.control.maxRpm        = 1500.0f;
    cfg.control.useExtSensor  = false;
    cfg.control.heaterEnabled = false;
    cfg.control.motorEnabled  = false;
    StirrerEncoder::_isrSteps = 0;
    StirrerControl::_pulseInterval = 0;
    control._config       = &cfg;
    control._tempPlate    = 25.0f;
    control._extConnected = false;
    display.writeCount    = 0;
    enc   = StirrerEncoder();
    menu  = StirrerMenu();
    enc.setup();
    menu.setup(&cfg, &control, &enc, &display);
}
void tearDown() {}

// Simulates a button press of the given length, starting now
void press(unsigned long heldMs) {
    mock_digitalRead_value = LOW;
    menu.update();
    mock_millis_value     += heldMs;
    mock_digitalRead_value = HIGH;
    menu.update();
}

void shortPresses(int count) {
    for (int i = 0; i < count; i++) {
        mock_millis_value += 100;
        press(150);
    }
}

void turn(int steps) {
    StirrerEncoder::_isrSteps = steps;
    menu.update();
}

// ── Page navigation ─────────────────────────────────────────
void test_short_press_advances_page() {
    shortPresses(1);
    TEST_ASSERT_TRUE(display.lastLine1.find("TEMP") != std::string::npos);
}

void test_page_wraps_around() {
    shortPresses(PAGE_COUNT);
    // Status page shows no square brackets
    TEST_ASSERT_TRUE(display.lastLine1.find("[") == std::string::npos);
}

// ── Change value ────────────────────────────────────────────
void test_rotate_increases_target_temp() {
    shortPresses(1);  // PAGE_TARGET_TEMP
    float before = cfg.control.targetTemp;
    turn(3);
    TEST_ASSERT_EQUAL_FLOAT(before + 3 * STEP_TEMP, cfg.control.targetTemp);
}

void test_rotate_decreases_target_temp() {
    shortPresses(1);  // PAGE_TARGET_TEMP
    float before = cfg.control.targetTemp;
    turn(-2);
    TEST_ASSERT_EQUAL_FLOAT(before - 2 * STEP_TEMP, cfg.control.targetTemp);
}

void test_rotate_increases_target_rpm() {
    shortPresses(2);  // PAGE_TARGET_RPM
    float before = cfg.control.targetRpm;
    turn(5);
    TEST_ASSERT_EQUAL_FLOAT(before + 5 * STEP_RPM, cfg.control.targetRpm);
}

// ── Limits ──────────────────────────────────────────────────
void test_temp_constrained_to_max() {
    shortPresses(1);  // PAGE_TARGET_TEMP
    cfg.control.targetTemp = 349.0f;
    turn(10);  // would exceed 350
    TEST_ASSERT_LESS_OR_EQUAL(350.0f, cfg.control.targetTemp);
}

void test_temp_constrained_to_min() {
    shortPresses(1);  // PAGE_TARGET_TEMP
    cfg.control.targetTemp = 1.0f;
    turn(-10);  // would go below 0
    TEST_ASSERT_GREATER_OR_EQUAL(0.0f, cfg.control.targetTemp);
}

void test_rpm_constrained_to_max_rpm() {
    shortPresses(2);  // PAGE_TARGET_RPM
    cfg.control.targetRpm = cfg.control.maxRpm - 5;
    turn(100);
    TEST_ASSERT_LESS_OR_EQUAL(cfg.control.maxRpm, cfg.control.targetRpm);
}

// ── Status page ──────────────────────────────────────────────
void test_status_page_shows_ist_and_soll() {
    control._tempPlate = 143.5f;
    // 287 rpm = 60e6 / interval µs
    StirrerControl::_pulseInterval = (unsigned long)(60000000.0f / 287.0f);
    StirrerControl::_lastPulseTime = 0;
    mock_micros_value  = 1;
    mock_millis_value += MENU_REFRESH_MS;
    menu.update();

    // Line 1 = measured values, line 2 = setpoints
    TEST_ASSERT_TRUE(display.lastLine1.find("143") != std::string::npos);
    TEST_ASSERT_TRUE(display.lastLine1.find("287") != std::string::npos);
    TEST_ASSERT_TRUE(display.lastLine2.find("100") != std::string::npos);  // targetTemp
    TEST_ASSERT_TRUE(display.lastLine2.find("300") != std::string::npos);  // targetRpm
}

void test_status_rotate_does_nothing() {
    float tempBefore = cfg.control.targetTemp;
    turn(5);
    TEST_ASSERT_EQUAL_FLOAT(tempBefore, cfg.control.targetTemp);
}

// ── Display content ─────────────────────────────────────────
void test_heater_page_shows_on_off() {
    shortPresses(5);  // PAGE_HEATER
    turn(-1);  // left = OFF
    TEST_ASSERT_TRUE(display.lastLine1.find("OFF") != std::string::npos);
    turn(1);   // right = ON
    TEST_ASSERT_TRUE(display.lastLine1.find("ON") != std::string::npos);
    TEST_ASSERT_TRUE(cfg.control.heaterEnabled);
}

// ── Redraw ───────────────────────────────────────────────────
void test_no_redraw_without_change() {
    int before = display.writeCount;
    for (int i = 0; i < 10; i++) {
        mock_millis_value += MENU_REFRESH_MS;
        menu.update();
    }
    TEST_ASSERT_EQUAL(before, display.writeCount);
}

void test_sensor_change_redrawn_after_refresh_interval() {
    int before = display.writeCount;
    control._tempPlate = 50.0f;
    mock_millis_value += 1;
    menu.update();
    TEST_ASSERT_EQUAL(before, display.writeCount);  // too early

    mock_millis_value += MENU_REFRESH_MS;
    menu.update();
    TEST_ASSERT_EQUAL(before + 1, display.writeCount);
    TEST_ASSERT_TRUE(display.lastLine1.find("50.0") != std::string::npos);
}

// ── Save ────────────────────────────────────────────────────
void test_long_press_shows_message_without_blocking() {
    mock_millis_value += 100;
    press(LONGPRESS_MS);
    TEST_ASSERT_EQUAL_STRING("Config saved!", display.lastLine1.c_str());

    mock_millis_value += MENU_MESSAGE_MS;
    menu.update();
    TEST_ASSERT_TRUE(display.lastLine1.find("rpm") != std::string::npos);
}

void test_long_press_writes_config() {
    cfg.control.targetTemp = 77.0f;
    mock_millis_value += 100;
    press(LONGPRESS_MS);

    StirrerConfig loaded;
    loaded.load();
    TEST_ASSERT_EQUAL_FLOAT(77.0f, loaded.control.targetTemp);
}

// ── Main ─────────────────────────────────────────────────────
int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_short_press_advances_page);
    RUN_TEST(test_page_wraps_around);
    RUN_TEST(test_rotate_increases_target_temp);
    RUN_TEST(test_rotate_decreases_target_temp);
    RUN_TEST(test_rotate_increases_target_rpm);
    RUN_TEST(test_temp_constrained_to_max);
    RUN_TEST(test_temp_constrained_to_min);
    RUN_TEST(test_rpm_constrained_to_max_rpm);
    RUN_TEST(test_status_page_shows_ist_and_soll);
    RUN_TEST(test_status_rotate_does_nothing);
    RUN_TEST(test_heater_page_shows_on_off);
    RUN_TEST(test_no_redraw_without_change);
    RUN_TEST(test_sensor_change_redrawn_after_refresh_interval);
    RUN_TEST(test_long_press_shows_message_without_blocking);
    RUN_TEST(test_long_press_writes_config);
    return UNITY_END();
}
