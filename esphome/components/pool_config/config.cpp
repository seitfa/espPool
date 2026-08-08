#include "config.h"

#include <esphome/core/preferences.h>
#include <algorithm>

namespace esphome {
namespace pool_controller {

PoolConfigComponent::PoolConfigComponent() = default;

void PoolConfigComponent::setup() {
  this->load_config();
}

void PoolConfigComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "PoolConfigComponent:");
  ESP_LOGCONFIG(TAG, "  target_ph: %.2f", this->target_ph_);
  ESP_LOGCONFIG(TAG, "  pool_volume_liters: %.0f", this->pool_volume_liters_);
  ESP_LOGCONFIG(TAG, "  hardness_mg_l: %.0f", this->hardness_mg_l_);
  ESP_LOGCONFIG(TAG, "  acid_type: %s", acid_type_to_string(this->acid_type_));
  ESP_LOGCONFIG(TAG, "  acid_strength_percent: %.2f", this->acid_strength_percent_);
  ESP_LOGCONFIG(TAG, "  dosing_flowrate_ml_per_min: %.2f", this->dosing_flowrate_ml_per_min_);
  ESP_LOGCONFIG(TAG, "  max_acid_ml_per_day: %.2f", this->max_acid_ml_per_day_);
  ESP_LOGCONFIG(TAG, "  acid_dosing_enabled: %s", this->acid_dosing_enabled_ ? "true" : "false");
  ESP_LOGCONFIG(TAG, "  pump_manual_disabled: %s", this->pump_manual_disabled_ ? "true" : "false");
}

void PoolConfigComponent::set_target_ph(float target_ph) {
  this->target_ph_ = target_ph;
  this->save_config();
}

void PoolConfigComponent::set_pool_volume_liters(float liters) {
  this->pool_volume_liters_ = liters;
  this->save_config();
}

void PoolConfigComponent::set_hardness_mg_l(float hardness) {
  this->hardness_mg_l_ = hardness;
  this->save_config();
}

void PoolConfigComponent::set_acid_type(AcidType acid_type) {
  this->acid_type_ = acid_type;
  this->save_config();
}

void PoolConfigComponent::set_acid_strength_percent(float strength) {
  this->acid_strength_percent_ = strength;
  this->save_config();
}

void PoolConfigComponent::set_dosing_flowrate_ml_per_min(float flowrate) {
  this->dosing_flowrate_ml_per_min_ = flowrate;
  this->save_config();
}

void PoolConfigComponent::set_max_acid_ml_per_day(float max_ml) {
  this->max_acid_ml_per_day_ = max_ml;
  this->save_config();
}

void PoolConfigComponent::set_acid_dosing_enabled(bool enabled) {
  this->acid_dosing_enabled_ = enabled;
  this->save_config();
}

void PoolConfigComponent::set_pump_manual_disabled(bool disabled) {
  this->pump_manual_disabled_ = disabled;
  this->save_config();
}

float PoolConfigComponent::calculate_acid_ml_needed(float current_ph) const {
  if (current_ph <= 0.0f || current_ph <= this->target_ph_ || this->pool_volume_liters_ <= 0.0f || this->acid_strength_percent_ <= 0.0f) {
    return 0.0f;
  }

  const float delta_ph = current_ph - this->target_ph_;
  const float hardness_factor = 1.0f + std::max(0.0f, this->hardness_mg_l_ - 100.0f) / 500.0f;
  const float strength_factor = std::max(0.01f, this->acid_strength_percent_) / 100.0f;
  const float base_ml_per_liter_per_ph = 0.04f;

  return delta_ph * this->pool_volume_liters_ * base_ml_per_liter_per_ph * hardness_factor / strength_factor;
}

float PoolConfigComponent::calculate_dosing_time_minutes(float acid_ml) const {
  if (acid_ml <= 0.0f || this->dosing_flowrate_ml_per_min_ <= 0.0f) {
    return 0.0f;
  }
  return acid_ml / this->dosing_flowrate_ml_per_min_;
}

void PoolConfigComponent::load_config() {
  // Default storage
  this->storage_ = PoolConfigStorage{};

  // Use esphome preferences API. Use a stable 32-bit key for this component's config.
  static constexpr uint32_t CONFIG_PREF_KEY = 0x50434647;  // 'PCFG'

  if (::esphome::global_preferences != nullptr) {
    auto pref = ::esphome::global_preferences->make_preference<PoolConfigStorage>(CONFIG_PREF_KEY);
    if (pref.load(&this->storage_) && this->storage_.magic == 0x50434F4E) {
      // Stored configuration loaded successfully.
    } else {
      this->storage_ = PoolConfigStorage{};
    }
  } else {
    // Preferences backend not available (e.g., unsupported platform), keep defaults.
    this->storage_ = PoolConfigStorage{};
  }

  this->target_ph_ = this->storage_.target_ph;
  this->pool_volume_liters_ = this->storage_.pool_volume_liters;
  this->hardness_mg_l_ = this->storage_.hardness_mg_l;
  this->acid_type_ = static_cast<AcidType>(this->storage_.acid_type);
  this->acid_strength_percent_ = this->storage_.acid_strength_percent;
  this->dosing_flowrate_ml_per_min_ = this->storage_.dosing_flowrate_ml_per_min;
  this->max_acid_ml_per_day_ = this->storage_.max_acid_ml_per_day;
  this->acid_dosing_enabled_ = this->storage_.acid_dosing_enabled;
  this->pump_manual_disabled_ = this->storage_.pump_manual_disabled;
}

void PoolConfigComponent::save_config() {
  this->storage_.target_ph = this->target_ph_;
  this->storage_.pool_volume_liters = this->pool_volume_liters_;
  this->storage_.hardness_mg_l = this->hardness_mg_l_;
  this->storage_.acid_type = static_cast<uint8_t>(this->acid_type_);
  this->storage_.acid_strength_percent = this->acid_strength_percent_;
  this->storage_.dosing_flowrate_ml_per_min = this->dosing_flowrate_ml_per_min_;
  this->storage_.max_acid_ml_per_day = this->max_acid_ml_per_day_;
  this->storage_.acid_dosing_enabled = this->acid_dosing_enabled_;
  this->storage_.pump_manual_disabled = this->pump_manual_disabled_;

  static constexpr uint32_t CONFIG_PREF_KEY = 0x50434647;  // 'PCFG'

  if (::esphome::global_preferences != nullptr) {
    auto pref = ::esphome::global_preferences->make_preference<PoolConfigStorage>(CONFIG_PREF_KEY);
    pref.save(&this->storage_);
    ::esphome::global_preferences->sync();
  }
}


}  // namespace pool_controller
}
