#pragma once

#include "esphome/core/component.h"
#include <string>

namespace esphome {
namespace switch_ {

class Switch {
 public:
    bool state{false};
    bool has_state{false};
    std::string name_{""};

    void set_name(const std::string &name) { name_ = name; }
    const std::string &get_name() const { return name_; }

    // Simuliert das Veröffentlichen des aktuellen Zustands nach außen
    void publish_state(bool value) {
        state = value;
        has_state = true;
    }

    /** Verarbeitet den Steuerungsbefehl (wird vom Frontend/Services aufgerufen)
     * Ruft am Ende die von dir implementierte write_state Methode auf.
     */
    void control(bool target_state) {
        this->write_state(target_state);
    }

    /** Schaltet diesen Switch EIN. Wird vom Front-End aufgerufen. */
    void turn_on() {
        this->control(true);
    }

    /** Schaltet diesen Switch AUS. Wird vom Front-End aufgerufen. */
    void turn_off() {
        this->control(false);
    }

    /** Invertiert den aktuellen Zustand dieses Switches. Wird vom Front-End aufgerufen. */
    void toggle() {
        this->control(!this->state);
    }

    // Diese Methode wird von ESPHome aufgerufen, wenn der Schalter hardwareseitig reagieren soll.
    // In deiner echten Komponente überschreibst du diese Methode.
    virtual void write_state(bool value) = 0;

    // Hilfsmethode für Unit-Tests zum Zurücksetzen der Testinstanz
    void mock_reset() {
        state = false;
        has_state = false;
    }
};

} // namespace switch_
} // namespace esphome