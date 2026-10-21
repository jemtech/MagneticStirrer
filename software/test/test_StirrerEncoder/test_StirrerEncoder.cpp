#include <unity.h>
#include <Arduino.h>
#include <Preferences.h>
#include <StirrerDisplay.h>
#define private public  // tests set the private ISR state
#include "StirrerEncoder.h"
#undef private

StirrerConfig  cfg;
StirrerDisplay display;

void setUp() {
    Preferences::resetAll();
    mock_digitalRead_value = HIGH;  // Button not pressed
    cfg.load();
    cfg.control.targetTemp    = 100.0f;
    cfg.control.targetRpm     = 300.0f;
    cfg.control.tempPlateMax  = 300.0f;
    cfg.control.maxRpm        = 1500.0f;
    cfg.control.useExtSensor  = false;
    cfg.control.heaterEnabled = false;
    cfg.control.motorEnabled  = false;
    StirrerEncoder::_isrSteps = 0;
    display.writeCount        = 0;
}
void tearDown() {}

// ── Parameter-Navigation ─────────────────────────────────────
void test_short_press_advances_param() {
    StirrerEncoder enc;
    enc.setup(&cfg, &display);
    // Startet bei PARAM_STATUS
    // Simuliere Kurzdruck: Button LOW dann HIGH
    mock_millis_value      = 100;
    mock_digitalRead_value = LOW;
    enc.update();
    mock_millis_value      = 250;  // 150ms gehalten – Kurzdruck
    mock_digitalRead_value = HIGH;
    enc.update();
    // Display sollte jetzt TEMP anzeigen
    std::string line1 = display.lastLine1;
    TEST_ASSERT_TRUE(line1.find("TEMP") != std::string::npos);
}

void test_param_wraps_around() {
    StirrerEncoder enc;
    enc.setup(&cfg, &display);
    // Press PARAM_COUNT times to wrap back to STATUS
    for (int i = 0; i < PARAM_COUNT; i++) {
        mock_millis_value      = 100 + i * 500;
        mock_digitalRead_value = LOW;
        enc.update();
        mock_millis_value      = 250 + i * 500;
        mock_digitalRead_value = HIGH;
        enc.update();
    }
    // Sollte wieder Statusseite sein
    enc.updateSensorValues(25.0f, 0.0f, false, 0.0f);
    // Statusseite zeigt keine eckigen Klammern
    TEST_ASSERT_TRUE(display.lastLine1.find("[") == std::string::npos);
}

// ── Change value ────────────────────────────────────────────
void test_rotate_increases_target_temp() {
    StirrerEncoder enc;
    enc.setup(&cfg, &display);

    // Zu PARAM_TARGET_TEMP wechseln
    mock_millis_value = 100; mock_digitalRead_value = LOW;  enc.update();
    mock_millis_value = 250; mock_digitalRead_value = HIGH; enc.update();

    float before = cfg.control.targetTemp;
    StirrerEncoder::_isrSteps = 3;  // 3 Schritte rechts
    enc.update();

    TEST_ASSERT_EQUAL_FLOAT(before + 3 * STEP_TEMP, cfg.control.targetTemp);
}

void test_rotate_decreases_target_temp() {
    StirrerEncoder enc;
    enc.setup(&cfg, &display);

    // Zu PARAM_TARGET_TEMP wechseln
    mock_millis_value = 100; mock_digitalRead_value = LOW;  enc.update();
    mock_millis_value = 250; mock_digitalRead_value = HIGH; enc.update();

    float before = cfg.control.targetTemp;
    StirrerEncoder::_isrSteps = -2;
    enc.update();

    TEST_ASSERT_EQUAL_FLOAT(before - 2 * STEP_TEMP, cfg.control.targetTemp);
}

void test_rotate_increases_target_rpm() {
    StirrerEncoder enc;
    enc.setup(&cfg, &display);

    // Navigate to PARAM_TARGET_RPM (press 2 times)
    for (int i = 0; i < 2; i++) {
        mock_millis_value = 100 + i*500; mock_digitalRead_value = LOW;  enc.update();
        mock_millis_value = 250 + i*500; mock_digitalRead_value = HIGH; enc.update();
    }

    float before = cfg.control.targetRpm;
    StirrerEncoder::_isrSteps = 5;
    enc.update();

    TEST_ASSERT_EQUAL_FLOAT(before + 5 * STEP_RPM, cfg.control.targetRpm);
}

// ── Grenzwerte ───────────────────────────────────────────────
void test_temp_constrained_to_max() {
    StirrerEncoder enc;
    enc.setup(&cfg, &display);

    // Zu PARAM_TARGET_TEMP wechseln
    mock_millis_value = 100; mock_digitalRead_value = LOW;  enc.update();
    mock_millis_value = 250; mock_digitalRead_value = HIGH; enc.update();

    cfg.control.targetTemp    = 349.0f;
    StirrerEncoder::_isrSteps = 10;  // would exceed 350
    enc.update();

    TEST_ASSERT_LESS_OR_EQUAL(350.0f, cfg.control.targetTemp);
}

void test_temp_constrained_to_min() {
    StirrerEncoder enc;
    enc.setup(&cfg, &display);

    // Zu PARAM_TARGET_TEMP wechseln
    mock_millis_value = 100; mock_digitalRead_value = LOW;  enc.update();
    mock_millis_value = 250; mock_digitalRead_value = HIGH; enc.update();

    cfg.control.targetTemp     = 1.0f;
    StirrerEncoder::_isrSteps  = -10;  // would go below 0
    enc.update();

    TEST_ASSERT_GREATER_OR_EQUAL(0.0f, cfg.control.targetTemp);
}

void test_rpm_constrained_to_max_rpm() {
    StirrerEncoder enc;
    enc.setup(&cfg, &display);

    // Zu PARAM_TARGET_RPM
    for (int i = 0; i < 2; i++) {
        mock_millis_value = 100 + i*500; mock_digitalRead_value = LOW;  enc.update();
        mock_millis_value = 250 + i*500; mock_digitalRead_value = HIGH; enc.update();
    }

    cfg.control.targetRpm      = cfg.control.maxRpm - 5;
    StirrerEncoder::_isrSteps  = 100;
    enc.update();

    TEST_ASSERT_LESS_OR_EQUAL(cfg.control.maxRpm, cfg.control.targetRpm);
}

// ── Status-Seite ─────────────────────────────────────────────
void test_status_page_shows_ist_and_soll() {
    StirrerEncoder enc;
    enc.setup(&cfg, &display);

    enc.updateSensorValues(143.5f, 0.0f, false, 287.0f);

    // Zeile 1 = Ist-Werte, Zeile 2 = Soll-Werte
    TEST_ASSERT_TRUE(display.lastLine1.find("143") != std::string::npos);
    TEST_ASSERT_TRUE(display.lastLine1.find("287") != std::string::npos);
    TEST_ASSERT_TRUE(display.lastLine2.find("100") != std::string::npos);  // targetTemp
    TEST_ASSERT_TRUE(display.lastLine2.find("300") != std::string::npos);  // targetRpm
}

void test_status_rotate_does_nothing() {
    StirrerEncoder enc;
    enc.setup(&cfg, &display);

    float tempBefore = cfg.control.targetTemp;
    StirrerEncoder::_isrSteps = 5;  // auf STATUS-Seite drehen
    enc.update();

    TEST_ASSERT_EQUAL_FLOAT(tempBefore, cfg.control.targetTemp);
}

// ── Display Inhalte ──────────────────────────────────────────
void test_heater_param_shows_on_off() {
    StirrerEncoder enc;
    enc.setup(&cfg, &display);

    // Navigate to PARAM_HEATER (press 5 times)
    for (int i = 0; i < 5; i++) {
        mock_millis_value = 100 + i*500; mock_digitalRead_value = LOW;  enc.update();
        mock_millis_value = 250 + i*500; mock_digitalRead_value = HIGH; enc.update();
    }

    cfg.control.heaterEnabled = false;
    StirrerEncoder::_isrSteps = -1;  // links = OFF
    enc.update();
    TEST_ASSERT_TRUE(display.lastLine1.find("OFF") != std::string::npos);

    StirrerEncoder::_isrSteps = 1;   // rechts = ON
    enc.update();
    TEST_ASSERT_TRUE(display.lastLine1.find("ON") != std::string::npos);
}

// ── Main ─────────────────────────────────────────────────────
int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_short_press_advances_param);
    RUN_TEST(test_param_wraps_around);
    RUN_TEST(test_rotate_increases_target_temp);
    RUN_TEST(test_rotate_decreases_target_temp);
    RUN_TEST(test_rotate_increases_target_rpm);
    RUN_TEST(test_temp_constrained_to_max);
    RUN_TEST(test_temp_constrained_to_min);
    RUN_TEST(test_rpm_constrained_to_max_rpm);
    RUN_TEST(test_status_page_shows_ist_and_soll);
    RUN_TEST(test_status_rotate_does_nothing);
    RUN_TEST(test_heater_param_shows_on_off);
    return UNITY_END();
}
