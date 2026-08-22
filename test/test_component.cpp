#include <unity.h>

#include "ph_controller.h"
#include "esphome/core/hal.h"
#include "esphome/components/gpio/gpio_switch.h"
#include "esphome/core/component.h"

esphome::pool_controller::PoolPhController ph_controller;
esphome::sensor::Sensor *ph_sensor, *used_today_sensor, *acid_needed_sensor;
esphome::InternalGPIOPin *ph_dosing_pump_pin;
esphome::gpio::GPIOSwitch *ph_dosing_pump;

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

    ph_controller.set_pool_volume_liters(50000.0f);
    ph_controller.set_tac_mg_l(100.0f);
    ph_controller.set_acid_strength_percent(14.9f);
    ph_controller.set_dosing_flowrate_ml_per_min(60.0f);
}

void tearDown(void) {}

void test_ph_dosing(void) {

    ph_controller.set_target_ph(7.0f); // Target pH is 7.2
    
    TEST_ASSERT_FALSE(ph_dosing_pump->state);
    ph_controller.set_current_ph(9.0f);
    TEST_ASSERT_EQUAL_FLOAT(9.0f, ph_sensor->state);


    ph_controller.set_current_ph(8.0f);
    ph_controller.update(); // This should trigger dosing logic
    TEST_ASSERT_EQUAL_FLOAT(4806.0f, acid_needed_sensor->state); // Check acid needed calculation
    
    ph_controller.set_current_ph(7.5f);
    ph_controller.update(); // This should trigger dosing logic
    TEST_ASSERT_EQUAL_FLOAT( 3487.0f, acid_needed_sensor->state); // Check acid needed calculation

    
    ph_controller.update(); // This should trigger dosing logic
    TEST_ASSERT_TRUE(ph_dosing_pump->state);

    float step = 1000 * 60 * 1; // 1 minutes in milliseconds
    esphome::MockTime::advance_time(step);
    
    ph_controller.set_current_ph(7.5f);
    ph_controller.update(); // This should trigger dosing logic
    TEST_ASSERT_TRUE(ph_dosing_pump->state);

    esphome::MockTime::advance_time(1000 * 60 * (40000 / 60.0f));
    ph_controller.set_current_ph(7.1f);
    ph_controller.update(); // This should trigger dosing logic
    TEST_ASSERT_FALSE(ph_dosing_pump->state);
}


int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_ph_dosing);
    return UNITY_END();
}