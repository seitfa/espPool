#include "pool_chemistry.h"

namespace pool_chemistry {

float PoolChemistry::calculate_acid_needed_ml(AcidType acid_type,
                                              float water_volume_liters,
                                              float current_ph,
                                              float target_ph,
                                              float tac) {
  if (water_volume_liters <= 0.0f || tac <= 0.0f) {
    return 0.0f;
  }

  if (current_ph <= 0.0f || target_ph <= 0.0f || current_ph <= target_ph) {
    return 0.0f;
  }

  const float tac_mmol_l = tac / 50.06f;

  const float current_bicarbonate_fraction =
      1.0f / (1.0f + std::pow(10.0f, current_ph - 6.35f));
  const float target_bicarbonate_fraction =
      1.0f / (1.0f + std::pow(10.0f, target_ph - 6.35f));
  const float alkalinity_shift = std::fabs(current_bicarbonate_fraction - target_bicarbonate_fraction);

  const float needed_mol_h = tac_mmol_l * alkalinity_shift * water_volume_liters / 1000.0f;

  const AcidParam acid = get_acid_param(acid_type);
  const float acid_volume_per_mol_h =
      acid.molar_mass / (acid.equivalents * acid.density * acid.concentration);

  return round(needed_mol_h * acid_volume_per_mol_h);
}

PoolChemistry::AcidParam PoolChemistry::get_acid_param(AcidType acid_type) {
  switch (acid_type) {
    case AcidType::SULFURIC_ACID_14_9: {
      AcidParam param;
      param.equivalents = 2.0f;
      param.molar_mass = 98.079f;
      param.density = 1.1013f;
      param.concentration = 0.149f;
      return param;
    }
    default: {
      AcidParam param{0.0f, 0.0f, 0.0f, 0.0f};
      return param;
    }
  }
}

}  // namespace pool_chemistry
