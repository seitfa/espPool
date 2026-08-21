#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace gpio {

class GPIOSwitch final : public switch_::Switch, public Component {
 public:
    GPIOPin *pin_{nullptr};

    void set_pin(GPIOPin *pin) { this->pin_ = pin; }
    
    void setup() override {
        if (this->pin_ != nullptr) {
            this->pin_->setup();
        }
    }

    void dump_config() override {}

 protected:
    // Schreibt den Zustand auf den simulierten Hardware-Pin
    void write_state(bool state) override {
        if (this->pin_ != nullptr) {
            this->pin_->digital_write(state);
        }
        this->publish_state(state);
    }
};

} // namespace gpio
} // namespace esphome