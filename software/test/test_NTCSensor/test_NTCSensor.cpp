#include <unity.h>
#include <Arduino.h>
#include "NTCSensor.h"

// ── Helper ──────────────────────────────────────────────────
// Calculates expected ADC value for a given temperature
// NTC 100k, B=3950, fixed resistor 22k
int tempToAdc(float tempC) {
    float tK    = tempC + 273.15f;
    float rNtc  = 100000.0f * exp(3950.0f * (1.0f / tK - 1.0f / 298.15f));
    float v     = 3.3f * rNtc / (22000.0f + rNtc);
    return (int)(v / 3.3f * 4095.0f);
}

// ── Tests ─────────────────────────────────────────────────────
void test_ntc_room_temperature() {
    mock_analogRead_value = tempToAdc(25.0f);
    NTCSensor ntc(34);
    ntc.setup();
    float temp = ntc.readTemp();
    TEST_ASSERT_FLOAT_WITHIN(2.0f, 25.0f, temp);
}

void test_ntc_high_temperature() {
    mock_analogRead_value = tempToAdc(150.0f);
    NTCSensor ntc(34);
    float temp = ntc.readTemp();
    TEST_ASSERT_FLOAT_WITHIN(5.0f, 150.0f, temp);
}

void test_ntc_low_temperature() {
    mock_analogRead_value = tempToAdc(0.0f);
    NTCSensor ntc(34);
    float temp = ntc.readTemp();
    TEST_ASSERT_FLOAT_WITHIN(2.0f, 0.0f, temp);
}

void test_ntc_connected() {
    mock_analogRead_value = 2048;  // mid-range – connected
    NTCSensor ntc(34);
    TEST_ASSERT_TRUE(ntc.isConnected());
}

void test_ntc_disconnected_high() {
    mock_analogRead_value = 4095;  // open – no NTC
    NTCSensor ntc(34);
    TEST_ASSERT_FALSE(ntc.isConnected());
}

void test_ntc_disconnected_low() {
    mock_analogRead_value = 0;  // short circuit
    NTCSensor ntc(34);
    TEST_ASSERT_FALSE(ntc.isConnected());
}

void test_ntc_invalid_voltage_returns_error() {
    mock_analogRead_value = 4095;  // 3.3V – division by zero guard
    NTCSensor ntc(34);
    float temp = ntc.readTemp();
    TEST_ASSERT_EQUAL_FLOAT(-999.0f, temp);
}

void test_ntc_custom_parameters() {
    // 10k NTC, B=3380 – different configuration
    NTCSensor ntc(34, 10000.0f, 10000.0f, 25.0f, 3380.0f);
    mock_analogRead_value = 2048;
    float temp = ntc.readTemp();
    // Must return a plausible value (not -999)
    TEST_ASSERT_NOT_EQUAL(-999.0f, temp);
}

// ── Main ──────────────────────────────────────────────────────
int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_ntc_room_temperature);
    RUN_TEST(test_ntc_high_temperature);
    RUN_TEST(test_ntc_low_temperature);
    RUN_TEST(test_ntc_connected);
    RUN_TEST(test_ntc_disconnected_high);
    RUN_TEST(test_ntc_disconnected_low);
    RUN_TEST(test_ntc_invalid_voltage_returns_error);
    RUN_TEST(test_ntc_custom_parameters);
    return UNITY_END();
}
