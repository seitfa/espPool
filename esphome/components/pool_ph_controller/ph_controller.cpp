#include "ph_controller.h"

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

#include <algorithm>

namespace esphome {
namespace pool_controller {

PoolPhController::PoolPhController(unsigned long update_interval_ms)
    : PollingComponent(update_interval_ms), pump_(nullptr) {}

void PoolPhController::setup() {
  const unsigned long now = millis();
  this->last_update_ms_ = now;
  this->last_daily_reset_ms_ = now;
}

void PoolPhController::update() {
  const unsigned long now = millis();
  if (this->pump_ && this->pump_->is_on()) {
    this->daily_on_ms_ += now - this->last_update_ms_;
  }
  this->reset_daily_usage_if_needed(now);
  this->control_ph();
  this->last_update_ms_ = now;
}

float PoolPhController::get_daily_acid_used_ml() const {
  return (this->daily_on_ms_ / 60000.0f) * this->dosing_flowrate_ml_per_min_;
}

void PoolPhController::reset_daily_usage_if_needed(unsigned long now) {
  if (now < this->last_update_ms_) {
    this->daily_on_ms_ = 0;
    this->dosing_end_ms_ = 0;
    this->last_daily_reset_ms_ = now;
  }
  if (now - this->last_daily_reset_ms_ >= DAY_MS) {
    this->daily_on_ms_ = 0;
    this->last_daily_reset_ms_ = now;
  }
}

void PoolPhController::control_ph() {
  const unsigned long now = millis();
  if (!this->pump_) {
    return;
  }

  const float current_ph = this->current_ph_;
  if (current_ph <= 0.0f || current_ph > 14.0f) {
    ESP_LOGW(TAG, "Invalid pH reading %.2f, disabling acid dosing", current_ph);
    this->error_disabled_ = true;
    if (this->pump_->is_on()) {
      this->pump_->turn_off();
    }
    return;
  }

  this->error_disabled_ = false;

  if (!this->acid_dosing_enabled_ || this->pump_manual_disabled_) {
    if (this->pump_->is_on()) {
      this->pump_->turn_off();
    }
    return;
  }

  const float delta_ph = current_ph - this->target_ph_;
  if (delta_ph <= PH_TOLERANCE) {
    if (this->pump_->is_on()) {
      this->pump_->turn_off();
    }
    return;
  }

  // Calculate acid needed using the same formula previously in PoolConfigComponent
  const float hardness_factor = 1.0f + std::max(0.0f, this->hardness_mg_l_ - 100.0f) / 500.0f;
  const float strength_factor = std::max(0.01f, this->acid_strength_percent_) / 100.0f;
  const float base_ml_per_liter_per_ph = 0.04f;
  const float acid_ml_needed = delta_ph * this->pool_volume_liters_ * base_ml_per_liter_per_ph * hardness_factor / strength_factor;

  const float used_today_ml = this->get_daily_acid_used_ml();
  const float remaining_ml = (this->max_acid_ml_per_day_ - used_today_ml > 0.0f) ? this->max_acid_ml_per_day_ - used_today_ml : 0.0f;

  if (acid_ml_needed <= 0.0f || remaining_ml <= 0.0f) {
    if (this->pump_->is_on()) {
      this->pump_->turn_off();
    }
    return;
  }

  const float deliverable_ml = (acid_ml_needed < remaining_ml) ? acid_ml_needed : remaining_ml;
  const float dosing_minutes = (this->dosing_flowrate_ml_per_min_ > 0.0f) ? (deliverable_ml / this->dosing_flowrate_ml_per_min_) : 0.0f;
  const unsigned long dosing_ms = static_cast<unsigned long>(dosing_minutes * 60000.0f);

  if (dosing_ms == 0) {
    if (this->pump_->is_on()) {
      this->pump_->turn_off();
    }
    return;
  }

  if (!this->pump_->is_on()) {
    ESP_LOGD(TAG, "Starting acid dosing for %.2f minutes to correct pH %.2f to target %.2f", dosing_minutes, current_ph, this->target_ph_);
    this->pump_->turn_on();
    this->dosing_end_ms_ = now + dosing_ms;
  }

  if (this->pump_->is_on() && now >= this->dosing_end_ms_) {
    ESP_LOGD(TAG, "Acid dosing period ended after %.2f minutes", dosing_minutes);
    this->pump_->turn_off();
  }
}

// Public setters

void PoolPhController::set_current_ph(float ph) {
  this->current_ph_ = ph;
  if (this->current_ph_sensor_) {
    this->current_ph_sensor_->publish_state(ph);
  }
  // React immediately to new reading
  this->control_ph();
}

void PoolPhController::set_target_ph(float target_ph) {
  this->target_ph_ = target_ph;
}

void PoolPhController::set_pool_volume_liters(float liters) {
  this->pool_volume_liters_ = liters;
}

void PoolPhController::set_hardness_mg_l(float hardness) {
  this->hardness_mg_l_ = hardness;
}

void PoolPhController::set_acid_strength_percent(float percent) {
  this->acid_strength_percent_ = percent;
}

void PoolPhController::set_dosing_flowrate_ml_per_min(float flowrate) {
  this->dosing_flowrate_ml_per_min_ = flowrate;
}

void PoolPhController::set_max_acid_ml_per_day(float max_ml) {
  this->max_acid_ml_per_day_ = max_ml;
}

void PoolPhController::set_acid_dosing_enabled(bool enabled) {
  this->acid_dosing_enabled_ = enabled;
  if (this->acid_dosing_enabled_sensor_) {
    this->acid_dosing_enabled_sensor_->publish_state(enabled);
  }
}

void PoolPhController::set_pump_manual_disabled(bool disabled) {
  this->pump_manual_disabled_ = disabled;
  if (this->pump_manual_disabled_sensor_) {
    this->pump_manual_disabled_sensor_->publish_state(disabled);
  }
  // If manual disabled, ensure pump is off
  if (disabled && this->pump_ && this->pump_->is_on()) {
    this->pump_->turn_off();
  }
}

void PoolPhController::set_acid_dosing_binary_sensor(binary_sensor::BinarySensor *b) {
  this->acid_dosing_enabled_sensor_ = b;
  if (b) {
    b->publish_state(this->acid_dosing_enabled_);
  }
}

void PoolPhController::set_pump_manual_disabled_binary_sensor(binary_sensor::BinarySensor *b) {
  this->pump_manual_disabled_sensor_ = b;
  if (b) {
    b->publish_state(this->pump_manual_disabled_);
  }
}

void PoolPhController::set_current_ph_sensor(sensor::Sensor *s) {
  this->current_ph_sensor_ = s;
  if (s) {
    s->publish_state(this->current_ph_);
  }
}

void PoolPhController::set_pump_switch(switch_::Switch *pump) {
  this->pump_ = pump;
}

}  // namespace pool_controller
}