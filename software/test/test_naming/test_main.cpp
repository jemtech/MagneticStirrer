#include <string.h>
#include <string>
#include <unity.h>
#include <StirrerNaming.h>

// Runs on the host: `pio test -e native`

static const uint8_t MAC[6] = {0x0A, 0xBC, 0x01, 0xFF, 0x10, 0x9E};

void setUp(void) {}
void tearDown(void) {}

void test_hostname_uses_stirrer_id(void) {
    char host[128];
    buildHostName(host, sizeof(host), "0", MAC);
    TEST_ASSERT_EQUAL_STRING("esp32stirrer_0", host);
}

void test_hostname_falls_back_to_mac_for_empty_id(void) {
    char host[128];
    buildHostName(host, sizeof(host), "", MAC);
    TEST_ASSERT_EQUAL_STRING("esp32stirrer_0A_BC_01_FF_10_9E", host);
}

void test_hostname_falls_back_to_mac_for_null_id(void) {
    char host[128];
    buildHostName(host, sizeof(host), NULL, MAC);
    TEST_ASSERT_EQUAL_STRING("esp32stirrer_0A_BC_01_FF_10_9E", host);
}

void test_hostname_truncates_long_id_and_fits_buffer(void) {
    std::string longId(300, 'x');
    char host[128];
    buildHostName(host, sizeof(host), longId.c_str(), MAC);
    TEST_ASSERT_EQUAL_STRING((std::string("esp32stirrer_") + std::string(110, 'x') + "").c_str(), host);
    TEST_ASSERT_LESS_OR_EQUAL(sizeof(host), strlen(host) + 1);
}

void test_hostname_never_overflows_small_buffer(void) {
    char host[8];
    buildHostName(host, sizeof(host), "abcdef", MAC);
    TEST_ASSERT_EQUAL_UINT(7, strlen(host));
    TEST_ASSERT_EQUAL_STRING("esp32st", host);
}

void test_hostname_handles_zero_size_buffer(void) {
    char host[1] = {'z'};
    buildHostName(host, 0, "a", MAC);
    TEST_ASSERT_EQUAL_CHAR('z', host[0]);
}

void test_ap_ssid_format(void) {
    char ssid[64];
    buildApSsid(ssid, sizeof(ssid), MAC);
    TEST_ASSERT_EQUAL_STRING("Stirrer-AP-0A:BC:01:FF:10:9E", ssid);
}

void test_ap_ssid_fits_80211_limit(void) {
    // 802.11 SSIDs are at most 32 bytes
    char ssid[64];
    buildApSsid(ssid, sizeof(ssid), MAC);
    TEST_ASSERT_LESS_OR_EQUAL_UINT(32, strlen(ssid));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_hostname_uses_stirrer_id);
    RUN_TEST(test_hostname_falls_back_to_mac_for_empty_id);
    RUN_TEST(test_hostname_falls_back_to_mac_for_null_id);
    RUN_TEST(test_hostname_truncates_long_id_and_fits_buffer);
    RUN_TEST(test_hostname_never_overflows_small_buffer);
    RUN_TEST(test_hostname_handles_zero_size_buffer);
    RUN_TEST(test_ap_ssid_format);
    RUN_TEST(test_ap_ssid_fits_80211_limit);
    return UNITY_END();
}
