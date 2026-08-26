#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

#include "pool_chemistry.h"

namespace esphome {
namespace pool_controller {

class PoolPhController : public PollingComponent {
 public:
  // Constructor: component created by codegen; pump switch attached via YAML config
  PoolPhController(unsigned long update_interval_ms = 15000);

  void setup() override;
  void update() override;

  // External API
  void set_current_ph(float ph);

  void set_target_ph(float target_ph);
  void set_pool_volume(float liters);
  void set_tac(float tac_mg_l);
  void set_acid_type(pool_chemistry::AcidType acid_type);
  void set_acid_strength_percent(float percent);
  void set_dosing_flowrate(float flowrate_ml_per_min);
  void set_max_acid(float max_ml);

  void set_acid_dosing_enabled(bool enabled);
  void set_pool_pump_running(bool running);
  void set_mixing_delay(float minutes);

  // Getters for config values (useful for template numbers)
  float get_target_ph() const { return this->target_ph_; }
  float get_pool_volume() const { return this->pool_volume_liters_; }
  float get_tac() const { return this->tac_mg_l_; }
  pool_chemistry::AcidType get_acid_type() const { return this->acid_type_; }
  float get_acid_strength_percent() const { return this->acid_strength_percent_; }
  float get_dosing_flowrate() const { return this->dosing_flowrate_ml_per_min_; }
  float get_max_acid() const { return this->max_acid_ml_per_day_; }
  float get_mixing_delay() const { return this->mixing_delay_minutes_; }
  bool is_acid_dosing_enabled() const { return this->acid_dosing_enabled_; }
  bool is_pool_pump_running() const { return this->pool_pump_running_; }

  // Sensor/binary sensors are owned by the component and exposed via codegen
  void set_acid_dosing_binary_sensor(binary_sensor::BinarySensor *b);
  void set_pool_pump_running_sensor(binary_sensor::BinarySensor *b);
  void set_mixing_delay_sensor(sensor::Sensor *s);
  void set_current_ph_sensor(sensor::Sensor *s);
  void set_target_ph_sensor(sensor::Sensor *s);
  void set_pool_volume_sensor(sensor::Sensor *s);
  void set_tac_sensor(sensor::Sensor *s);
  void set_acid_strength_sensor(sensor::Sensor *s);
  void set_dosing_flowrate_sensor(sensor::Sensor *s);
  void set_max_acid_sensor(sensor::Sensor *s);
  void set_daily_acid_used_ml_sensor(sensor::Sensor *s);
  void set_acid_ml_needed_sensor(sensor::Sensor *s);

  // Attach pump switch (from YAML using use_id)
  void set_pump_switch(switch_::Switch *pump);

  float get_daily_acid_used_ml() const;
  bool is_error_disabled() const { return this->error_disabled_; }

 private:
  // configuration values (defaults preserved from previous PoolConfigStorage)
  float target_ph_ = 7.2f;
  float pool_volume_liters_ = 50000.0f;
  float tac_mg_l_ = 150.0f;
  pool_chemistry::AcidType acid_type_ = pool_chemistry::AcidType::SULFURIC_ACID_14_9_PERCENT;
  float acid_strength_percent_ = 14.9f;
  float dosing_flowrate_ml_per_min_ = 60.0f;
  float max_acid_ml_per_day_ = 2000.0f;
  float mixing_delay_minutes_ = 5.0f;
  bool acid_dosing_enabled_ = true;
  bool pool_pump_running_ = false;

  // runtime state
  float current_ph_ = 0.0f;

  switch_::Switch *pump_{nullptr};
  unsigned long daily_on_ms_ = 0;
  unsigned long dosing_end_ms_ = 0;
  unsigned long last_update_ms_ = 0;
  unsigned long last_daily_reset_ms_ = 0;
  unsigned long pool_pump_started_ms_ = 0;
  unsigned long last_dosing_started_ms_ = 0;
  unsigned long last_dosing_end_ms_ = 0;
  bool error_disabled_ = false;

  // optional sensors to expose states
  binary_sensor::BinarySensor *acid_dosing_enabled_sensor_{nullptr};
  binary_sensor::BinarySensor *pool_pump_running_sensor_{nullptr};
  sensor::Sensor *current_ph_sensor_{nullptr};
  sensor::Sensor *target_ph_sensor_{nullptr};
  sensor::Sensor *pool_volume_sensor_{nullptr};
  sensor::Sensor *tac_sensor_{nullptr};
  sensor::Sensor *acid_strength_sensor_{nullptr};
  sensor::Sensor *dosing_flowrate_sensor_{nullptr};
  sensor::Sensor *max_acid_sensor_{nullptr};
  sensor::Sensor *mixing_delay_sensor_{nullptr};
  sensor::Sensor *used_today_ml_sensor_{nullptr};
  sensor::Sensor *acid_ml_needed_sensor_{nullptr};

  static constexpr const char *TAG = "pool_controller.ph";
  static constexpr unsigned long DAY_MS = 24UL * 60UL * 60UL * 1000UL;
  static constexpr float PH_TOLERANCE = 0.05f;

  void reset_daily_usage_if_needed(unsigned long now);
  void control_ph();
  void publish_internal_state_sensors();
};

}  // namespace pool_controller
}
  