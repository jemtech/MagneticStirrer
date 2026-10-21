#include <unity.h>
#include <Arduino.h>
#define private public  // tests set the private ISR state
#include "StirrerEncoder.h"
#undef private

void setUp() {
    mock_digitalRead_value    = HIGH;  // Button not pressed
    mock_millis_value         = 0;
    StirrerEncoder::_isrSteps = 0;
}
void tearDown() {}

// Button LOW at pressAt, HIGH at releaseAt; returns the press reported on release
EncoderPress press(StirrerEncoder& enc, unsigned long pressAt, unsigned long releaseAt) {
    mock_millis_value      = pressAt;
    mock_digitalRead_value = LOW;
    TEST_ASSERT_EQUAL(PRESS_NONE, enc.takePress());
    mock_millis_value      = releaseAt;
    mock_digitalRead_value = HIGH;
    return enc.takePress();
}

// ── Rotation ─────────────────────────────────────────────────
void test_take_steps_returns_and_clears() {
    StirrerEncoder enc;
    enc.setup();
    StirrerEncoder::_isrSteps = 3;
    TEST_ASSERT_EQUAL(3, enc.takeSteps());
    TEST_ASSERT_EQUAL(0, enc.takeSteps());
}

void test_take_steps_negative() {
    StirrerEncoder enc;
    enc.setup();
    StirrerEncoder::_isrSteps = -2;
    TEST_ASSERT_EQUAL(-2, enc.takeSteps());
}

// ── Button ───────────────────────────────────────────────────
void test_no_press() {
    StirrerEncoder enc;
    enc.setup();
    TEST_ASSERT_EQUAL(PRESS_NONE, enc.takePress());
}

void test_short_press() {
    StirrerEncoder enc;
    enc.setup();
    TEST_ASSERT_EQUAL(PRESS_SHORT, press(enc, 100, 250));
}

void test_long_press() {
    StirrerEncoder enc;
    enc.setup();
    TEST_ASSERT_EQUAL(PRESS_LONG, press(enc, 100, 100 + LONGPRESS_MS));
}

void test_bounce_is_ignored() {
    StirrerEncoder enc;
    enc.setup();
    TEST_ASSERT_EQUAL(PRESS_NONE, press(enc, 100, 100 + BTN_DEBOUNCE_MS - 1));
}

void test_press_reported_once() {
    StirrerEncoder enc;
    enc.setup();
    TEST_ASSERT_EQUAL(PRESS_SHORT, press(enc, 100, 250));
    TEST_ASSERT_EQUAL(PRESS_NONE, enc.takePress());
}

// ── Main ─────────────────────────────────────────────────────
int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_take_steps_returns_and_clears);
    RUN_TEST(test_take_steps_negative);
    RUN_TEST(test_no_press);
    RUN_TEST(test_short_press);
    RUN_TEST(test_long_press);
    RUN_TEST(test_bounce_is_ignored);
    RUN_TEST(test_press_reported_once);
    return UNITY_END();
}
