#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

class MyCustomSensor : public esphome::Component, public esphome::sensor::Sensor {
 public:
    float calculate_linear_value(float input) {
        return (input * 1.8f) + 32.0f; // Example hardware-free logic
    }
    
    void update_logic(float raw_value) {
        float processed = calculate_linear_value(raw_value);
        publish_state(processed);
    }
};