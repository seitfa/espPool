#include "ph_controller.h"

#include <algorithm>

namespace pool_controller {

PoolPhController::PoolPhController(PoolConfigComponent *config, AtlasEzoPhSensor *probe, switch_::Switch *pump, unsigned long update_interval_ms)
    : PollingComponent(update_interval_ms), config_(config), probe_(probe), pump_(pump), daily_on_ms_(0), dosing_end_ms_(0), last_update_ms_(0), last_daily_reset_ms_(0), error_disabled_(false) {}

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
  this->last_update_ms_ = now;
}

float PoolPhController::get_daily_acid_used_ml() const {
  if (!this->config_) {
    return 0.0f;
  }
  return (this->daily_on_ms_ / 60000.0f) * this->config_->get_dosing_flowrate_ml_per_min();
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
  if (!this->config_ || !this->probe_ || !this->pump_) {
    return;
  }

  const float current_ph = this->probe_->get_latest_ph();
  if (current_ph <= 0.0f || current_ph > 14.0f) {
    ESP_LOGW(TAG, "Invalid pH reading %.2f, disabling acid dosing", current_ph);
    this->error_disabled_ = true;
    if (this->pump_->is_on()) {
      this->pump_->turn_off();
    }
    return;
  }

  this->error_disabled_ = false;

  if (!this->config_->is_acid_dosing_enabled() || this->config_->is_pump_manual_disabled()) {
    if (this->pump_->is_on()) {
      this->pump_->turn_off();
    }
    return;
  }

  const float delta_ph = current_ph - this->config_->get_target_ph();
  if (delta_ph <= PH_TOLERANCE) {
    if (this->pump_->is_on()) {
      this->pump_->turn_off();
    }
    return;
  }

  const float acid_ml_needed = this->config_->calculate_acid_ml_needed(current_ph);
  const float used_today_ml = this->get_daily_acid_used_ml();
  const float remaining_ml = std::max(0.0f, this->config_->get_max_acid_ml_per_day() - used_today_ml);

  if (acid_ml_needed <= 0.0f || remaining_ml <= 0.0f) {
    if (this->pump_->is_on()) {
      this->pump_->turn_off();
    }
    return;
  }

  const float deliverable_ml = std::min(acid_ml_needed, remaining_ml);
  const float dosing_minutes = this->config_->calculate_dosing_time_minutes(deliverable_ml);
  const unsigned long dosing_ms = static_cast<unsigned long>(dosing_minutes * 60000.0f);

  if (dosing_ms == 0) {
    if (this->pump_->is_on()) {
      this->pump_->turn_off();
    }
    return;
  }

  if (!this->pump_->is_on()) {
    ESP_LOGD(TAG, "Starting acid dosing for %.2f minutes to correct pH %.2f to target %.2f", dosing_minutes, current_ph, this->config_->get_target_ph());
    this->pump_->turn_on();
    this->dosing_end_ms_ = now + dosing_ms;
  }

  if (this->pump_->is_on() && now >= this->dosing_end_ms_) {
    ESP_LOGD(TAG, "Acid dosing period ended after %.2f minutes", dosing_minutes);
    this->pump_->turn_off();
  }
}

}  // namespace pool_controller

}  // namespace pool_controller
