#pragma once

namespace esphome {

class GPIOPin {
 public:
    bool current_value{false};

    virtual void setup() {}
    virtual void digital_write(bool value) {
        current_value = value;
    }
    virtual bool digital_read() {
        return current_value;
    }
};

// Subklasse für Output-Pins (wird von GPIOSwitch erwartet)
class InternalGPIOPin : public GPIOPin {};

} // namespace esphome