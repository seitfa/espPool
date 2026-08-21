#pragma once

#include <string>
#include <cstdint>

namespace esphome {
    // Eine globale Struktur, die den aktuellen Zeitstempel im Test hält
    struct MockTime {
        static inline uint32_t current_millis{0};
        static inline uint32_t current_micros{0};
        
        // Hilfsfunktion, um die Zeit im Test vorzuspringen
        static void advance_time(uint32_t ms) {
            current_millis += ms;
            current_micros += (ms * 1000);
        }

        // Setzt die Zeit für den nächsten Testlauf zurück
        static void reset() {
            current_millis = 0;
            current_micros = 0;
        }
    };
}

// Überschreibt die globalen ESPHome / Arduino Zeitfunktionen
inline uint32_t millis() { return esphome::MockTime::current_millis; }
inline uint32_t micros() { return esphome::MockTime::current_micros; }

// Simuliert die ESPHome-Logging-Makros
#define ESP_LOGD(tag, ...) (void)0
#define ESP_LOGI(tag, ...) (void)0
#define ESP_LOGW(tag, ...) (void)0
#define ESP_LOGE(tag, ...) (void)0

namespace esphome {

enum ComponentStatus {
    STATUS_NONE = 0,
    STATUS_SETUP = 1,
    STATUS_LOOP = 2,
    STATUS_FAILED = 3
};

class Component {
 public:
    virtual void setup() {}
    virtual void loop() {}
    virtual void dump_config() {}
    virtual float get_setup_priority() { return 0.0f; }
    
    void status_set_warning() { status_ = STATUS_FAILED; }
    void status_clear_warning() { status_ = STATUS_NONE; }
    bool is_failed() const { return status_ == STATUS_FAILED; }

 protected:
    ComponentStatus status_{STATUS_NONE};
};

// --- HIER BEGINNT DER POLLING_COMPONENT MOCK ---
class PollingComponent : public Component {
 public:
    PollingComponent() : update_interval_(1000) {}
    explicit PollingComponent(uint32_t update_interval) : update_interval_(update_interval) {}

    // Diese Methode implementiert deine eigene Komponente
    virtual void update() = 0;

    // Erlaubt es ESPHome (oder dem YAML), das Intervall zu setzen/auszulesen
    void set_update_interval(uint32_t update_interval) { this_target_time_changed_ = true; update_interval_ = update_interval; }
    uint32_t get_update_interval() const { return this->update_interval_; }

    // HILFSMETHODE FÜR DEINE UNIT-TESTS:
    // Simuliert, dass die Zeit abgelaufen ist und ruft manuell update() auf
    void mock_trigger_update() {
        this->update();
    }

 protected:
    uint32_t update_interval_;
    bool this_target_time_changed_{false};
};

} // namespace esphome