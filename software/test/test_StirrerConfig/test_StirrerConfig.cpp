#include <unity.h>
#include "../mocks/ArduinoMock.h"
#include "../mocks/PreferencesMock.h"
#include "../mocks/ArduinoJsonMock.h"
#include "StirrerConfig.h"

int           mock_analogRead_value  = 0;
int           mock_digitalRead_value = 0;
unsigned long mock_millis_value      = 0;
unsigned long mock_micros_value      = 0;

void setUp() {
    Preferences::clearAll();
}
void tearDown() {}

// ── WiFi Tests ────────────────────────────────────────────────
void test_config_wifi_defaults() {
    StirrerConfig cfg;
    cfg.load();
    TEST_ASSERT_EQUAL_STRING("", cfg.wifi.ssid.c_str());
    TEST_ASSERT_EQUAL_STRING("", cfg.wifi.password.c_str());
}

void test_config_wifi_write_and_read() {
    StirrerConfig cfg;
    cfg.wifi.ssid     = "MyNetwork";
    cfg.wifi.password = "secret123";
    cfg.write();

    StirrerConfig cfg2;
    cfg2.load();
    TEST_ASSERT_EQUAL_STRING("MyNetwork", cfg2.wifi.ssid.c_str());
    TEST_ASSERT_EQUAL_STRING("secret123", cfg2.wifi.password.c_str());
}

// ── MQTT Tests ────────────────────────────────────────────────
void test_config_mqtt_defaults() {
    StirrerConfig cfg;
    cfg.load();
    TEST_ASSERT_EQUAL_STRING("", cfg.mqtt.ip.c_str());
    TEST_ASSERT_EQUAL_STRING("", cfg.mqtt.user.c_str());
    TEST_ASSERT_EQUAL_STRING("", cfg.mqtt.password.c_str());
}

void test_config_mqtt_write_and_read() {
    StirrerConfig cfg;
    cfg.mqtt.ip       = "192.168.1.100";
    cfg.mqtt.user     = "admin";
    cfg.mqtt.password = "mqttpass";
    cfg.write();

    StirrerConfig cfg2;
    cfg2.load();
    TEST_ASSERT_EQUAL_STRING("192.168.1.100", cfg2.mqtt.ip.c_str());
    TEST_ASSERT_EQUAL_STRING("admin",         cfg2.mqtt.user.c_str());
    TEST_ASSERT_EQUAL_STRING("mqttpass",      cfg2.mqtt.password.c_str());
}

// ── Control Tests ─────────────────────────────────────────────
void test_config_control_defaults() {
    StirrerConfig cfg;
    cfg.load();
    TEST_ASSERT_EQUAL_FLOAT(25.0f,   cfg.control.targetTemp);
    TEST_ASSERT_EQUAL_FLOAT(300.0f,  cfg.control.tempPlateMax);
    TEST_ASSERT_EQUAL_FLOAT(300.0f,  cfg.control.targetRpm);
    TEST_ASSERT_EQUAL_FLOAT(1500.0f, cfg.control.maxRpm);
    TEST_ASSERT_FALSE(cfg.control.useExtSensor);
    TEST_ASSERT_FALSE(cfg.control.heaterEnabled);
    TEST_ASSERT_FALSE(cfg.control.motorEnabled);
}

void test_config_control_write_and_read() {
    StirrerConfig cfg;
    cfg.load();
    cfg.control.targetTemp    = 120.0f;
    cfg.control.tempPlateMax  = 250.0f;
    cfg.control.targetRpm     = 600.0f;
    cfg.control.maxRpm        = 1200.0f;
    cfg.control.useExtSensor  = true;
    cfg.control.heaterEnabled = true;
    cfg.control.motorEnabled  = true;
    cfg.write();

    StirrerConfig cfg2;
    cfg2.load();
    TEST_ASSERT_EQUAL_FLOAT(120.0f,  cfg2.control.targetTemp);
    TEST_ASSERT_EQUAL_FLOAT(250.0f,  cfg2.control.tempPlateMax);
    TEST_ASSERT_EQUAL_FLOAT(600.0f,  cfg2.control.targetRpm);
    TEST_ASSERT_EQUAL_FLOAT(1200.0f, cfg2.control.maxRpm);
    TEST_ASSERT_TRUE(cfg2.control.useExtSensor);
    TEST_ASSERT_TRUE(cfg2.control.heaterEnabled);
    TEST_ASSERT_TRUE(cfg2.control.motorEnabled);
}

// ── PID Tests ────────────────────────────────────────────────
void test_config_pid_defaults() {
    StirrerConfig cfg;
    cfg.load();
    TEST_ASSERT_EQUAL_DOUBLE(5.0,  cfg.control.pidPlate.kp);
    TEST_ASSERT_EQUAL_DOUBLE(0.2,  cfg.control.pidPlate.ki);
    TEST_ASSERT_EQUAL_DOUBLE(2.0,  cfg.control.pidPlate.kd);
    TEST_ASSERT_EQUAL_DOUBLE(3.0,  cfg.control.pidExt.kp);
    TEST_ASSERT_EQUAL_DOUBLE(0.5,  cfg.control.pidExt.ki);
    TEST_ASSERT_EQUAL_DOUBLE(1.0,  cfg.control.pidExt.kd);
    TEST_ASSERT_EQUAL_DOUBLE(0.5,  cfg.control.pidMotor.kp);
    TEST_ASSERT_EQUAL_DOUBLE(0.1,  cfg.control.pidMotor.ki);
    TEST_ASSERT_EQUAL_DOUBLE(0.05, cfg.control.pidMotor.kd);
}

void test_config_pid_write_and_read() {
    StirrerConfig cfg;
    cfg.load();
    cfg.control.pidPlate  = { 4.0, 0.3, 1.5 };
    cfg.control.pidExt    = { 2.5, 0.4, 0.8 };
    cfg.control.pidMotor  = { 1.0, 0.2, 0.1 };
    cfg.write();

    StirrerConfig cfg2;
    cfg2.load();
    TEST_ASSERT_EQUAL_DOUBLE(4.0, cfg2.control.pidPlate.kp);
    TEST_ASSERT_EQUAL_DOUBLE(0.3, cfg2.control.pidPlate.ki);
    TEST_ASSERT_EQUAL_DOUBLE(1.5, cfg2.control.pidPlate.kd);
    TEST_ASSERT_EQUAL_DOUBLE(2.5, cfg2.control.pidExt.kp);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, cfg2.control.pidMotor.kp);
    TEST_ASSERT_EQUAL_DOUBLE(0.1, cfg2.control.pidMotor.kd);
}

// ── Boundary Tests ────────────────────────────────────────────
void test_config_temp_boundary_values() {
    StirrerConfig cfg;
    cfg.load();
    cfg.control.targetTemp   = 0.0f;
    cfg.control.tempPlateMax = 350.0f;
    cfg.write();

    StirrerConfig cfg2;
    cfg2.load();
    TEST_ASSERT_EQUAL_FLOAT(0.0f,   cfg2.control.targetTemp);
    TEST_ASSERT_EQUAL_FLOAT(350.0f, cfg2.control.tempPlateMax);
}

void test_config_rpm_boundary_values() {
    StirrerConfig cfg;
    cfg.load();
    cfg.control.targetRpm = 0.0f;
    cfg.control.maxRpm    = 3000.0f;
    cfg.write();

    StirrerConfig cfg2;
    cfg2.load();
    TEST_ASSERT_EQUAL_FLOAT(0.0f,    cfg2.control.targetRpm);
    TEST_ASSERT_EQUAL_FLOAT(3000.0f, cfg2.control.maxRpm);
}

// ── Main ─────────────────────────────────────────────────────
int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_config_wifi_defaults);
    RUN_TEST(test_config_wifi_write_and_read);
    RUN_TEST(test_config_mqtt_defaults);
    RUN_TEST(test_config_mqtt_write_and_read);
    RUN_TEST(test_config_control_defaults);
    RUN_TEST(test_config_control_write_and_read);
    RUN_TEST(test_config_pid_defaults);
    RUN_TEST(test_config_pid_write_and_read);
    RUN_TEST(test_config_temp_boundary_values);
    RUN_TEST(test_config_rpm_boundary_values);
    return UNITY_END();
}
