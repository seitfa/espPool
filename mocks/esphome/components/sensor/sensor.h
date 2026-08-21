#pragma once

#include "esphome/core/component.h"
#include <string>
#include <vector>

namespace esphome {
namespace sensor {

class Sensor {
 public:
    float state{0.0f};
    bool has_state{false};
    std::string name_{""};

    void set_name(const std::string &name) { name_ = name; }
    const std::string &get_name() const { return name_; }

    // Simuliert das Veröffentlichen eines neuen Messwerts
    void publish_state(float value) {
        state = value;
        has_state = true;
    }

    // Hilfsmethode für deine Unit-Tests zum Zurücksetzen
    void mock_reset() {
        state = 0.0f;
        has_state = false;
    }
};

} // namespace sensor
} // namespace esphome