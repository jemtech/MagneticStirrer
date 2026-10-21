#include <string>
#include <unity.h>
#include <Preferences.h>
#include <StirrerConfig.h>

// Runs on the host: `pio test -e native`

void setUp(void) { Preferences::resetAll(); }
void tearDown(void) {}

static std::string toJsonString(StirrerConfig& config) {
    JsonDocument doc = config.toJson();
    std::string out;
    serializeJson(doc, out);
    return out;
}

void test_public_json_omits_passwords(void) {
    StirrerConfig config;
    config.wifi.ssid = "my-network";
    config.wifi.password = "s3cret";
    config.mqtt.ip = "192.168.1.10";
    config.mqtt.user = "stirrer";
    config.mqtt.password = "mqtt-pw";

    JsonDocument doc = config.toPublicJson();

    TEST_ASSERT_EQUAL_STRING("my-network", doc["wifi"]["ssid"]);
    TEST_ASSERT_FALSE(doc["wifi"]["password"].is<const char*>());
    TEST_ASSERT_EQUAL_STRING("192.168.1.10", doc["mqtt"]["ip"]);
    TEST_ASSERT_EQUAL_STRING("stirrer", doc["mqtt"]["user"]);
    TEST_ASSERT_FALSE(doc["mqtt"]["password"].is<const char*>());
    TEST_ASSERT_EQUAL_FLOAT(25.0f, doc["control"]["targetTemp"].as<float>());

    std::string out;
    serializeJson(doc, out);
    TEST_ASSERT_TRUE(out.find("s3cret") == std::string::npos);
    TEST_ASSERT_TRUE(out.find("mqtt-pw") == std::string::npos);
}

void test_load_without_stored_config_uses_empty_defaults(void) {
    StirrerConfig config;
    config.load();

    TEST_ASSERT_EQUAL_STRING("", config.wifi.ssid.c_str());
    TEST_ASSERT_EQUAL_STRING("", config.wifi.password.c_str());
    TEST_ASSERT_EQUAL_STRING("", config.mqtt.ip.c_str());
    TEST_ASSERT_EQUAL_STRING("", config.mqtt.user.c_str());
    TEST_ASSERT_EQUAL_STRING("", config.mqtt.password.c_str());
}

void test_load_persists_defaults(void) {
    StirrerConfig config;
    config.load();

    Preferences prefs;
    prefs.begin("wifi", true);
    TEST_ASSERT_TRUE(prefs.isKey("ssid"));
    prefs.end();
    prefs.begin("mqtt", true);
    TEST_ASSERT_TRUE(prefs.isKey("ip"));
    prefs.end();
}

void test_write_then_load_roundtrip(void) {
    StirrerConfig writer;
    writer.wifi.ssid = "my-network";
    writer.wifi.password = "s3cret";
    writer.mqtt.ip = "192.168.1.10";
    writer.mqtt.user = "stirrer";
    writer.mqtt.password = "mqtt-pw";
    writer.write();

    StirrerConfig reader;
    reader.load();

    TEST_ASSERT_EQUAL_STRING("my-network", reader.wifi.ssid.c_str());
    TEST_ASSERT_EQUAL_STRING("s3cret", reader.wifi.password.c_str());
    TEST_ASSERT_EQUAL_STRING("192.168.1.10", reader.mqtt.ip.c_str());
    TEST_ASSERT_EQUAL_STRING("stirrer", reader.mqtt.user.c_str());
    TEST_ASSERT_EQUAL_STRING("mqtt-pw", reader.mqtt.password.c_str());
}

void test_write_overwrites_previous_values(void) {
    StirrerConfig config;
    config.wifi.ssid = "first";
    config.write();
    config.wifi.ssid = "second";
    config.write();

    StirrerConfig reader;
    reader.load();
    TEST_ASSERT_EQUAL_STRING("second", reader.wifi.ssid.c_str());
}

void test_load_keeps_existing_values_and_does_not_reset(void) {
    StirrerConfig writer;
    writer.wifi.ssid = "keepme";
    writer.mqtt.ip = "1.2.3.4";
    writer.write();

    StirrerConfig first;
    first.load();
    StirrerConfig second;
    second.load();

    TEST_ASSERT_EQUAL_STRING("keepme", second.wifi.ssid.c_str());
    TEST_ASSERT_EQUAL_STRING("1.2.3.4", second.mqtt.ip.c_str());
}

void test_to_json_contains_all_fields(void) {
    StirrerConfig config;
    config.wifi.ssid = "net";
    config.wifi.password = "pw";
    config.mqtt.ip = "10.0.0.1";
    config.mqtt.user = "u";
    config.mqtt.password = "p";

    JsonDocument doc = config.toJson();

    TEST_ASSERT_EQUAL_STRING("net", doc["wifi"]["ssid"]);
    TEST_ASSERT_EQUAL_STRING("pw", doc["wifi"]["password"]);
    TEST_ASSERT_EQUAL_STRING("10.0.0.1", doc["mqtt"]["ip"]);
    TEST_ASSERT_EQUAL_STRING("u", doc["mqtt"]["user"]);
    TEST_ASSERT_EQUAL_STRING("p", doc["mqtt"]["password"]);
}

void test_to_json_serializes_to_expected_json(void) {
    StirrerConfig config;
    config.wifi.ssid = "net";
    config.wifi.password = "pw";
    config.mqtt.ip = "10.0.0.1";
    config.mqtt.user = "u";
    config.mqtt.password = "p";

    std::string json = toJsonString(config);
    TEST_ASSERT_EQUAL_STRING(
        "{\"wifi\":{\"ssid\":\"net\",\"password\":\"pw\"},"
        "\"mqtt\":{\"ip\":\"10.0.0.1\",\"user\":\"u\",\"password\":\"p\"},",
        json.substr(0, json.find("\"control\"")).c_str());
}

void test_special_characters_survive_roundtrip_and_json(void) {
    StirrerConfig writer;
    writer.wifi.ssid = "caf\xC3\xA9 \"quoted\" & spaces";
    writer.wifi.password = "p@ss/w\\rd";
    writer.write();

    StirrerConfig reader;
    reader.load();
    TEST_ASSERT_EQUAL_STRING(writer.wifi.ssid.c_str(), reader.wifi.ssid.c_str());
    TEST_ASSERT_EQUAL_STRING(writer.wifi.password.c_str(), reader.wifi.password.c_str());

    JsonDocument parsed;
    TEST_ASSERT_TRUE(deserializeJson(parsed, toJsonString(reader)) == DeserializationError::Ok);
    TEST_ASSERT_EQUAL_STRING(writer.wifi.ssid.c_str(), parsed["wifi"]["ssid"]);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_public_json_omits_passwords);
    RUN_TEST(test_load_without_stored_config_uses_empty_defaults);
    RUN_TEST(test_load_persists_defaults);
    RUN_TEST(test_write_then_load_roundtrip);
    RUN_TEST(test_write_overwrites_previous_values);
    RUN_TEST(test_load_keeps_existing_values_and_does_not_reset);
    RUN_TEST(test_to_json_contains_all_fields);
    RUN_TEST(test_to_json_serializes_to_expected_json);
    RUN_TEST(test_special_characters_survive_roundtrip_and_json);
    return UNITY_END();
}
