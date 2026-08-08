#pragma once

#pragma once

#include "esphome.h"
#include "config.h"
#include "atlas_ezo_ph.h"

namespace pool_controller {

class PoolPhController : public PollingComponent {
 public:
  PoolPhController(PoolConfigComponent *config, AtlasEzoPhSensor *probe, switch_::Switch *pump, unsigned long update_interval_ms = 15000);

  void setup() override;
  void update() override;

  float get_daily_acid_used_ml() const;
  bool is_error_disabled() const { return this->error_disabled_; }

 private:
  PoolConfigComponent *config_{nullptr};
  AtlasEzoPhSensor *probe_{nullptr};
  switch_::Switch *pump_{nullptr};
  unsigned long daily_on_ms_;
  unsigned long dosing_end_ms_;
  unsigned long last_update_ms_;
  unsigned long last_daily_reset_ms_;
  bool error_disabled_;

  static constexpr const char *TAG = "pool_controller.ph";
  static constexpr unsigned long DAY_MS = 24UL * 60UL * 60UL * 1000UL;
  static constexpr float PH_TOLERANCE = 0.05f;

  void reset_daily_usage_if_needed(unsigned long now);
  void control_ph();
};

}  // namespace pool_controller
