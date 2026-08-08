#pragma once

#include "esphome.h"
#include <Wire.h>
#include <string>

namespace pool_controller {

class AtlasEzoPhSensor : public PollingComponent, public Sensor {
 public:
  explicit AtlasEzoPhSensor(uint8_t address = 0x63, unsigned long update_interval_ms = 15000);

  void setup() override;
  void update() override;

  float get_latest_ph() const;

 private:
  uint8_t address_;
  float latest_ph_;
  bool waiting_for_response_;
  unsigned long request_sent_ms_;

  std::string read_response();
  static constexpr const char *TAG = "pool_controller.ezo_ph";
};

}  // namespace pool_controller
