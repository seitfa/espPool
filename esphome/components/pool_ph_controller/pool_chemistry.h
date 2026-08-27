#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace pool_chemistry {

enum class AcidType : uint8_t {
  SULFURIC_ACID_14_9 = 0,
};

class PoolChemistry {
 public:
  struct AcidParam {
    float equivalents;     // acid equivalents per mole (e.g., H2SO4 => 2)
    float molar_mass;      // g/mol
    float density;         // g/mL
    float concentration;   // fraction (0..1) representing % w/w or similar
  };

  static float calculate_acid_needed_ml(AcidType acid_type,
                                        float water_volume_liters,
                                        float current_ph,
                                        float target_ph,
                                        float tac);

  static AcidParam get_acid_param(AcidType acid_type);
};
}  // namespace pool_chemistry
