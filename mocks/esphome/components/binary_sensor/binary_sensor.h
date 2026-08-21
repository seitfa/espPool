#pragma once

#include "esphome/core/component.h"
#include <string>
#include <vector>

namespace esphome {
namespace binary_sensor {

class BinarySensor {
 public:
    bool state{false};
    bool has_state{false};
    std::string name_{""};

    void set_name(const std::string &name) { name_ = name; }
    const std::string &get_name() const { return name_; }

    // Simuliert das Veröffentlichen eines neuen Boolean-Zustands (true/false)
    void publish_state(bool value) {
        state = value;
        has_state = true;
    }

    // Simuliert das Invertieren des aktuellen Zustands
    void publish_initial_state(bool value) {
        publish_state(value);
    }

    // Hilfsmethode für deine Unit-Tests zum Zurücksetzen des Testzustands
    void mock_reset() {
        state = false;
        has_state = false;
    }
};

} // namespace binary_sensor
} // namespace esphome