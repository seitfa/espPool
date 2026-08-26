#include "ph_controller.h"

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

#include "pool_chemistry.h"

#include <algorithm>
#include <cmath>

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
  if (this->pump_ && this->pump_->state) {
    this->daily_on_ms_ += now - this->last_update_ms_;
  }
  this->reset_daily_usage_if_needed(now);
  this->control_ph();
  this->publish_internal_state_sensors();
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

void PoolPhController::publish_internal_state_sensors() {
  if (this->target_ph_sensor_) {
    this->target_ph_sensor_->publish_state(this->target_ph_);
  }
  if (this->pool_volume_sensor_) {
    this->pool_volume_sensor_->publish_state(this->pool_volume_liters_);
  }
  if (this->tac_sensor_) {
    this->tac_sensor_->publish_state(this->tac_mg_l_);
  }
  if (this->acid_strength_sensor_) {
    this->acid_strength_sensor_->publish_state(this->acid_strength_percent_);
  }
  if (this->dosing_flowrate_sensor_) {
    this->dosing_flowrate_sensor_->publish_state(this->dosing_flowrate_ml_per_min_);
  }
  if (this->max_acid_sensor_) {
    this->max_acid_sensor_->publish_state(this->max_acid_ml_per_day_);
  }
  if (this->acid_dosing_enabled_sensor_) {
    this->acid_dosing_enabled_sensor_->publish_state(this->acid_dosing_enabled_);
  }
  if (this->pool_pump_running_sensor_) {
    this->pool_pump_running_sensor_->publish_state(this->pool_pump_running_);
  }
  if (this->mixing_delay_sensor_) {
    this->mixing_delay_sensor_->publish_state(this->mixing_delay_minutes_);
  }
  if (this->current_ph_sensor_) {
    this->current_ph_sensor_->publish_state(this->current_ph_);
  }
  if (this->used_today_ml_sensor_) {
    this->used_today_ml_sensor_->publish_state(this->get_daily_acid_used_ml());
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
    if (this->pump_->state) {
      this->pump_->turn_off();
    }
    return;
  }

  this->error_disabled_ = false;

  if (!this->acid_dosing_enabled_) {
    if (this->pump_->state) {
      this->pump_->turn_off();
    }
    return;
  }

  const float delta_ph = current_ph - this->target_ph_;
  if (delta_ph <= PH_TOLERANCE) {
    if (this->pump_->state) {
      this->pump_->turn_off();
    }
    return;
  }

  const float acid_needed_ml = pool_chemistry::PoolChemistry::calculate_acid_needed_ml(
      this->acid_type_, this->pool_volume_liters_, current_ph, this->target_ph_, this->tac_mg_l_);

  const float used_today_ml = this->get_daily_acid_used_ml();
  // publish computed values to optional sensors
  if (this->used_today_ml_sensor_) {
    this->used_today_ml_sensor_->publish_state(used_today_ml);
  }
  if (this->acid_ml_needed_sensor_) {
    this->acid_ml_needed_sensor_->publish_state(acid_needed_ml);
  }

  const float remaining_ml = (this->max_acid_ml_per_day_ - used_today_ml > 0.0f) ? this->max_acid_ml_per_day_ - used_today_ml : 0.0f;

  if (acid_needed_ml <= 0.0f || remaining_ml <= 0.0f) {
    if (this->pump_->state) {
      this->pump_->turn_off();
    }
    return;
  }

  const float deliverable_ml = (acid_needed_ml < remaining_ml) ? acid_needed_ml : remaining_ml;
  const float dosing_minutes = (this->dosing_flowrate_ml_per_min_ > 0.0f) ? (deliverable_ml / this->dosing_flowrate_ml_per_min_) : 0.0f;
  const unsigned long dosing_ms = static_cast<unsigned long>(dosing_minutes * 60000.0f);
  const unsigned long mixing_delay_ms = static_cast<unsigned long>(this->mixing_delay_minutes_ * 60000.0f);

  if (dosing_ms == 0) {
    if (this->pump_->state) {
      this->pump_->turn_off();
    }
    return;
  }

  if (this->pump_->state && now >= this->dosing_end_ms_) {
    ESP_LOGD(TAG, "Acid dosing period ended after %.2f minutes", dosing_minutes);
    this->pump_->turn_off();
    this->last_dosing_end_ms_ = now;
    return;
  }

  const bool pump_running_long_enough = this->pool_pump_running_ &&
                                        (this->pool_pump_started_ms_ != 0) &&
                                        (now - this->pool_pump_started_ms_ >= mixing_delay_ms);
  const bool enough_time_since_last_dose = this->last_dosing_end_ms_ == 0 ||
                                          (now - this->last_dosing_end_ms_ >= mixing_delay_ms);

  if (!this->pump_->state) {
    if (!this->pool_pump_running_ || !pump_running_long_enough || !enough_time_since_last_dose) {
      return;
    }
    ESP_LOGD(TAG, "Starting acid dosing for %.2f minutes to correct pH %.2f to target %.2f", dosing_minutes, current_ph, this->target_ph_);
    this->pump_->turn_on();
    this->last_dosing_started_ms_ = now;
    this->dosing_end_ms_ = now + dosing_ms;
  }
}

// Public setters

void PoolPhController::set_current_ph(float ph) {
  this->current_ph_ = ph;
  if (this->current_ph_sensor_) {
    this->current_ph_sensor_->publish_state(ph);
  }
  // Do not run control logic here. Control is executed from update()
  // to avoid side-effects during sensor callbacks and to match test
  // expectations where dosing is triggered only on update().
}

void PoolPhController::set_target_ph(float target_ph) {
  this->target_ph_ = target_ph;
  if (this->target_ph_sensor_) {
    this->target_ph_sensor_->publish_state(target_ph);
  }
}

void PoolPhController::set_pool_volume(float liters) {
  this->pool_volume_liters_ = liters;
  if (this->pool_volume_sensor_) {
    this->pool_volume_sensor_->publish_state(liters);
  }
}

void PoolPhController::set_tac(float tac) {
  this->tac_mg_l_ = tac;
  if (this->tac_sensor_) {
    this->tac_sensor_->publish_state(tac);
  }
}

void PoolPhController::set_acid_type(pool_chemistry::AcidType acid_type) {
  this->acid_type_ = acid_type;
}

void PoolPhController::set_acid_strength_percent(float percent) {
  this->acid_strength_percent_ = percent;
  if (this->acid_strength_sensor_) {
    this->acid_strength_sensor_->publish_state(percent);
  }
}

void PoolPhController::set_dosing_flowrate(float flowrate) {
  this->dosing_flowrate_ml_per_min_ = flowrate;
  if (this->dosing_flowrate_sensor_) {
    this->dosing_flowrate_sensor_->publish_state(flowrate);
  }
}

void PoolPhController::set_max_acid(float max_ml) {
  this->max_acid_ml_per_day_ = max_ml;
  if (this->max_acid_sensor_) {
    this->max_acid_sensor_->publish_state(max_ml);
  }
}

void PoolPhController::set_acid_dosing_enabled(bool enabled) {
  this->acid_dosing_enabled_ = enabled;
  if (this->acid_dosing_enabled_sensor_) {
    this->acid_dosing_enabled_sensor_->publish_state(enabled);
  }
}

void PoolPhController::set_pool_pump_running(bool running) {
  if (this->pool_pump_running_ != running) {
    this->pool_pump_running_ = running;
    if (running) {
      this->pool_pump_started_ms_ = millis();
    } else {
      this->pool_pump_started_ms_ = 0;
    }
  } else if (running && this->pool_pump_started_ms_ == 0) {
    this->pool_pump_started_ms_ = millis();
  }

  if (this->pool_pump_running_sensor_) {
    this->pool_pump_running_sensor_->publish_state(running);
  }
}

void PoolPhController::set_mixing_delay(float minutes) {
  this->mixing_delay_minutes_ = minutes;
  if (this->mixing_delay_sensor_) {
    this->mixing_delay_sensor_->publish_state(minutes);
  }
}

void PoolPhController::set_acid_dosing_binary_sensor(binary_sensor::BinarySensor *b) {
  this->acid_dosing_enabled_sensor_ = b;
  if (b) {
    b->publish_state(this->acid_dosing_enabled_);
  }
}

void PoolPhController::set_pool_pump_running_sensor(binary_sensor::BinarySensor *b) {
  this->pool_pump_running_sensor_ = b;
  if (b) {
    b->publish_state(this->pool_pump_running_);
  }
}

void PoolPhController::set_mixing_delay_sensor(sensor::Sensor *s) {
  this->mixing_delay_sensor_ = s;
  if (s) {
    s->publish_state(this->mixing_delay_minutes_);
  }
}

void PoolPhController::set_current_ph_sensor(sensor::Sensor *s) {
  this->current_ph_sensor_ = s;
  if (s) {
    s->publish_state(this->current_ph_);
  }
}

void PoolPhController::set_target_ph_sensor(sensor::Sensor *s) {
  this->target_ph_sensor_ = s;
  if (s) {
    s->publish_state(this->target_ph_);
  }
}

void PoolPhController::set_pool_volume_sensor(sensor::Sensor *s) {
  this->pool_volume_sensor_ = s;
  if (s) {
    s->publish_state(this->pool_volume_liters_);
  }
}

void PoolPhController::set_tac_sensor(sensor::Sensor *s) {
  this->tac_sensor_ = s;
  if (s) {
    s->publish_state(this->tac_mg_l_);
  }
}

void PoolPhController::set_acid_strength_sensor(sensor::Sensor *s) {
  this->acid_strength_sensor_ = s;
  if (s) {
    s->publish_state(this->acid_strength_percent_);
  }
}

void PoolPhController::set_dosing_flowrate_sensor(sensor::Sensor *s) {
  this->dosing_flowrate_sensor_ = s;
  if (s) {
    s->publish_state(this->dosing_flowrate_ml_per_min_);
  }
}

void PoolPhController::set_max_acid_sensor(sensor::Sensor *s) {
  this->max_acid_sensor_ = s;
  if (s) {
    s->publish_state(this->max_acid_ml_per_day_);
  }
}

void PoolPhController::set_daily_acid_used_ml_sensor(sensor::Sensor *s) {
  this->used_today_ml_sensor_ = s;
  if (s) {
    s->publish_state(this->get_daily_acid_used_ml());
  }
}

void PoolPhController::set_acid_ml_needed_sensor(sensor::Sensor *s) {
  this->acid_ml_needed_sensor_ = s;
  if (s) {
    // publish an initial 0; actual computed value will be published during control_ph()
    s->publish_state(0.0f);
  }
}

void PoolPhController::set_pump_switch(switch_::Switch *pump) {
  this->pump_ = pump;
}

}  // namespace pool_controller
}