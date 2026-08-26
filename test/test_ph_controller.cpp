#include <unity.h>

#include "ph_controller.h"
#include "esphome/core/hal.h"
#include "esphome/components/gpio/gpio_switch.h"
#include "esphome/core/component.h"

esphome::pool_controller::PoolPhController ph_controller;
esphome::sensor::Sensor *ph_sensor, *used_today_sensor, *acid_needed_sensor;
esphome::InternalGPIOPin *ph_dosing_pump_pin;
esphome::gpio::GPIOSwitch *ph_dosing_pump;

const long MIN_IN_MS = 1000 * 60;

void setUp(void) {
    
    ph_sensor = new esphome::sensor::Sensor();
    used_today_sensor = new esphome::sensor::Sensor();
    acid_needed_sensor = new esphome::sensor::Sensor();
    
    ph_dosing_pump_pin = new esphome::InternalGPIOPin();
    ph_dosing_pump = new esphome::gpio::GPIOSwitch();
    ph_dosing_pump->set_pin(ph_dosing_pump_pin);
    ph_dosing_pump->setup();

    ph_controller = esphome::pool_controller::PoolPhController();
    ph_controller.set_pump_switch(ph_dosing_pump);
    ph_controller.set_current_ph_sensor(ph_sensor);
    ph_controller.set_daily_acid_used_ml_sensor(used_today_sensor);
    ph_controller.set_acid_ml_needed_sensor(acid_needed_sensor);

    ph_controller.set_pool_volume(50000.0f);
    ph_controller.set_tac(100.0f);
    ph_controller.set_dosing_flowrate(75.0f);
    ph_controller.set_mixing_delay(10.0f); // Set mixing delay to 10 minutes
}

void tearDown(void) {}

void advance_time_and_update(unsigned long mins) {
    esphome::MockTime::advance_time(mins * 60000);
    ph_controller.update();
}

void test_ph_acid_calculation(void) {

    ph_controller.set_target_ph(7.0f); // Target pH is 7.0

    // Check acid needed calculation for 7.1 to 7.0
    ph_controller.set_current_ph(7.1f);
    ph_controller.update();
    TEST_ASSERT_EQUAL_INT(953, acid_needed_sensor->state); // Check acid needed calculation

    // Check acid needed calculation for 7.5 to 7.0
    ph_controller.set_current_ph(7.5f);
    ph_controller.update();
    TEST_ASSERT_EQUAL_INT(3487, acid_needed_sensor->state); // Check acid needed calculation

    // Check acid needed calculation for 8.0 to 7.0
    ph_controller.set_current_ph(8.0f);
    ph_controller.update();
    TEST_ASSERT_EQUAL_INT(4806, acid_needed_sensor->state); // Check acid needed calculation
}

void test_ph_mixing_delay(void) {
    ph_controller.set_target_ph(7.0f);
    ph_controller.set_current_ph(7.1f);
    ph_controller.set_pool_pump_running(false);

    advance_time_and_update(1);
    TEST_ASSERT_FALSE(ph_dosing_pump->state);

    advance_time_and_update(15);
    TEST_ASSERT_FALSE(ph_dosing_pump->state);

    // Check initial mixing delay behavior
    ph_controller.set_pool_pump_running(true);
    for(int i = 1; i < 10; ++i) {
        advance_time_and_update(1);
        TEST_ASSERT_FALSE(ph_dosing_pump->state);
    }
    // Check that dosing runs for dosing time and stops after that
    for(int i = 0; i < 953 / 75; ++i) {
        advance_time_and_update(1);
        TEST_ASSERT_TRUE(ph_dosing_pump->state);
    }
    advance_time_and_update(2);
    TEST_ASSERT_FALSE(ph_dosing_pump->state);
}


int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_ph_acid_calculation);
    RUN_TEST(test_ph_mixing_delay);
    return UNITY_END();
}