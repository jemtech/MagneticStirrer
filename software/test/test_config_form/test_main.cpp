#include <map>
#include <string>
#include <unity.h>
#include <StirrerConfig.h>

// Runs on the host: `pio test -e native`

typedef std::map<std::string, std::string> Form;

static StirrerConfig::FormLookup lookupIn(const Form& form) {
    return [&form](const char* name, String& value) {
        auto it = form.find(name);
        if (it == form.end()) return false;
        value = it->second.c_str();
        return true;
    };
}

static Form completeForm() {
    return {{"wifiSsid", "net"}, {"wifiPwd", "pw"}, {"mqttServerIp", "10.0.0.1"},
            {"mqttUser", "u"},   {"mqttPwd", "p"}};
}

void setUp(void) {}
void tearDown(void) {}

void test_complete_form_fills_config(void) {
    StirrerConfig config;
    Form form = completeForm();

    TEST_ASSERT_NULL(config.applyForm(lookupIn(form)));

    TEST_ASSERT_EQUAL_STRING("net", config.wifi.ssid.c_str());
    TEST_ASSERT_EQUAL_STRING("pw", config.wifi.password.c_str());
    TEST_ASSERT_EQUAL_STRING("10.0.0.1", config.mqtt.ip.c_str());
    TEST_ASSERT_EQUAL_STRING("u", config.mqtt.user.c_str());
    TEST_ASSERT_EQUAL_STRING("p", config.mqtt.password.c_str());
}

void test_each_missing_field_is_reported_by_name(void) {
    const char* names[] = {"wifiSsid", "wifiPwd", "mqttServerIp", "mqttUser", "mqttPwd"};
    for (const char* name : names) {
        StirrerConfig config;
        Form form = completeForm();
        form.erase(name);
        TEST_ASSERT_EQUAL_STRING_MESSAGE(name, config.applyForm(lookupIn(form)), name);
    }
}

void test_first_missing_field_wins(void) {
    StirrerConfig config;
    Form form;
    TEST_ASSERT_EQUAL_STRING("wifiSsid", config.applyForm(lookupIn(form)));
}

void test_empty_values_are_accepted(void) {
    StirrerConfig config;
    Form form = completeForm();
    form["mqttUser"] = "";
    form["mqttPwd"] = "";

    TEST_ASSERT_NULL(config.applyForm(lookupIn(form)));
    TEST_ASSERT_EQUAL_STRING("", config.mqtt.user.c_str());
}

void test_applied_form_serializes_to_json(void) {
    StirrerConfig config;
    Form form = completeForm();
    config.applyForm(lookupIn(form));

    JsonDocument doc = config.toJson();
    TEST_ASSERT_EQUAL_STRING("net", doc["wifi"]["ssid"]);
    TEST_ASSERT_EQUAL_STRING("10.0.0.1", doc["mqtt"]["ip"]);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_complete_form_fills_config);
    RUN_TEST(test_each_missing_field_is_reported_by_name);
    RUN_TEST(test_first_missing_field_wins);
    RUN_TEST(test_empty_values_are_accepted);
    RUN_TEST(test_applied_form_serializes_to_json);
    return UNITY_END();
}
