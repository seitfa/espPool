#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace esphome {
namespace pool_chemistry {

enum class AcidType : uint8_t {
  SULFURIC_ACID_14_9_PERCENT = 0,
};

class PoolChemistry {
 public:
  static float calculate_acid_needed_ml(AcidType acid_type,
                                        float water_volume_liters,
                                        float current_ph,
                                        float target_ph,
                                        float tac);

 private:
  static float get_acid_equivalent_factor(AcidType acid_type);
  static float get_acid_molar_mass(AcidType acid_type);
  static float get_acid_density(AcidType acid_type);
  static float get_acid_concentration(AcidType acid_type);
};

}  // namespace pool_chemistry
}  // namespace esphome
