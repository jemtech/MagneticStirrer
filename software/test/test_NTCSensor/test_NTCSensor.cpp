#include <unity.h>
#include "../mocks/ArduinoMock.h"
#include "NTCSensor.h"

// Mock-Werte
int           mock_analogRead_value  = 2048;
int           mock_digitalRead_value = LOW;
unsigned long mock_millis_value      = 0;
unsigned long mock_micros_value      = 0;

// ── Hilfsfunktion ────────────────────────────────────────────
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
    mock_analogRead_value = 2048;  // Mittelwert – verbunden
    NTCSensor ntc(34);
    TEST_ASSERT_TRUE(ntc.isConnected());
}

void test_ntc_disconnected_high() {
    mock_analogRead_value = 4095;  // offen – kein NTC
    NTCSensor ntc(34);
    TEST_ASSERT_FALSE(ntc.isConnected());
}

void test_ntc_disconnected_low() {
    mock_analogRead_value = 0;  // Kurzschluss
    NTCSensor ntc(34);
    TEST_ASSERT_FALSE(ntc.isConnected());
}

void test_ntc_invalid_voltage_returns_error() {
    mock_analogRead_value = 4095;  // 3.3V – Division by zero Schutz
    NTCSensor ntc(34);
    float temp = ntc.readTemp();
    TEST_ASSERT_EQUAL_FLOAT(-999.0f, temp);
}

void test_ntc_custom_parameters() {
    // 10k NTC, B=3380 – andere Konfiguration
    NTCSensor ntc(34, 10000.0f, 10000.0f, 25.0f, 3380.0f);
    mock_analogRead_value = 2048;
    float temp = ntc.readTemp();
    // Muss einen sinnvollen Wert liefern (nicht -999)
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
