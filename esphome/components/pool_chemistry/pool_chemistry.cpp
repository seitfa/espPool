#include "pool_chemistry.h"

namespace esphome {
namespace pool_controller {

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

  // Moles of H+ needed to consume the alkalinity that corresponds to the pH shift.
  const float needed_mol_h = tac_mmol_l * alkalinity_shift * water_volume_liters / 1000.0f;

  const float acid_equivalents = get_acid_equivalent_factor(acid_type);
  const float acid_molar_mass = get_acid_molar_mass(acid_type);
  const float acid_density = get_acid_density(acid_type);
  const float acid_concentration = get_acid_concentration(acid_type);

  // Volume per mole of H+ equivalents = molar_mass / (equivalents * density * concentration)
  const float acid_volume_per_mol_h = 
      acid_molar_mass / (acid_equivalents * acid_density * acid_concentration);

  return round(needed_mol_h * acid_volume_per_mol_h);
}

float PoolChemistry::get_acid_equivalent_factor(AcidType acid_type) {
  switch (acid_type) {
    case AcidType::SULFURIC_ACID_14_9_PERCENT:
      return 2.0f;
    default:
      return 0.0f;
  }
}

float PoolChemistry::get_acid_molar_mass(AcidType acid_type) {
  switch (acid_type) {
    case AcidType::SULFURIC_ACID_14_9_PERCENT:
      return 98.079f;
    default:
      return 0.0f;
  }
}

float PoolChemistry::get_acid_density(AcidType acid_type) {
  switch (acid_type) {
    case AcidType::SULFURIC_ACID_14_9_PERCENT:
      return 1.1013f;
    default:
      return 0.0f;
  }
}

float PoolChemistry::get_acid_concentration(AcidType acid_type) {
  switch (acid_type) {
    case AcidType::SULFURIC_ACID_14_9_PERCENT:
      return 0.149f;
    default:
      return 0.0f;
  }
}

}  // namespace pool_controller
}  // namespace esphome
