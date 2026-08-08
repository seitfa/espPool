#pragma once

#include "esphome.h"
#include <algorithm>
#include <Preferences.h>

namespace pool_controller {

enum class AcidType : uint8_t {
  MURIATIC_ACID = 0,
  SULFURIC_ACID = 1,
  PHOSPHORIC_ACID = 2,
};

inline const char *acid_type_to_string(AcidType type) {
  switch (type) {
    case AcidType::MURIATIC_ACID:
      return "Muriatic Acid";
    case AcidType::SULFURIC_ACID:
      return "Sulfuric Acid";
    case AcidType::PHOSPHORIC_ACID:
      return "Phosphoric Acid";
    default:
      return "Unknown Acid";
  }
}

struct PoolConfigStorage {
  uint32_t magic = 0x50434F4E;  // "PCON"
  float target_ph = 7.2f;
  float pool_volume_liters = 50000.0f;
  float hardness_mg_l = 150.0f;
  uint8_t acid_type = static_cast<uint8_t>(AcidType::MURIATIC_ACID);
  float acid_strength_percent = 31.45f;
  float dosing_flowrate_ml_per_min = 60.0f;
  float max_acid_ml_per_day = 2000.0f;
  bool acid_dosing_enabled = true;
  bool pump_manual_disabled = false;
};

class PoolConfigComponent : public Component {
 public:
  PoolConfigComponent();
  void setup() override;
  void dump_config() override;

  float get_target_ph() const { return this->target_ph_; }
  void set_target_ph(float target_ph);

  float get_pool_volume_liters() const { return this->pool_volume_liters_; }
  void set_pool_volume_liters(float liters);

  float get_hardness_mg_l() const { return this->hardness_mg_l_; }
  void set_hardness_mg_l(float hardness);

  AcidType get_acid_type() const { return this->acid_type_; }
  void set_acid_type(AcidType acid_type);

  float get_acid_strength_percent() const { return this->acid_strength_percent_; }
  void set_acid_strength_percent(float strength);

  float get_dosing_flowrate_ml_per_min() const { return this->dosing_flowrate_ml_per_min_; }
  void set_dosing_flowrate_ml_per_min(float flowrate);

  float get_max_acid_ml_per_day() const { return this->max_acid_ml_per_day_; }
  void set_max_acid_ml_per_day(float max_ml);

  bool is_acid_dosing_enabled() const { return this->acid_dosing_enabled_; }
  void set_acid_dosing_enabled(bool enabled);

  bool is_pump_manual_disabled() const { return this->pump_manual_disabled_; }
  void set_pump_manual_disabled(bool disabled);

  float calculate_acid_ml_needed(float current_ph) const;
  float calculate_dosing_time_minutes(float acid_ml) const;

 private:
  static constexpr const char *TAG = "pool_controller.config";
  PoolConfigStorage storage_{};
  float target_ph_ = 7.2f;
  float pool_volume_liters_ = 50000.0f;
  float hardness_mg_l_ = 150.0f;
  AcidType acid_type_ = AcidType::MURIATIC_ACID;
  float acid_strength_percent_ = 31.45f;
  float dosing_flowrate_ml_per_min_ = 60.0f;
  float max_acid_ml_per_day_ = 2000.0f;
  bool acid_dosing_enabled_ = true;
  bool pump_manual_disabled_ = false;

  void load_config();
  void save_config();
};

}  // namespace pool_controller
