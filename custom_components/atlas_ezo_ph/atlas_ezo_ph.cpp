#include "atlas_ezo_ph.h"

#include <stdlib.h>

namespace pool_controller {

AtlasEzoPhSensor::AtlasEzoPhSensor(uint8_t address, unsigned long update_interval_ms)
    : PollingComponent(update_interval_ms), Sensor(), address_(address), latest_ph_(NAN), waiting_for_response_(false), request_sent_ms_(0) {}

void AtlasEzoPhSensor::setup() {
  this->waiting_for_response_ = false;
  this->latest_ph_ = NAN;
}

void AtlasEzoPhSensor::update() {
  if (this->waiting_for_response_) {
    const std::string response = this->read_response();
    if (!response.empty()) {
      const float parsed_value = static_cast<float>(atof(response.c_str()));
      if (parsed_value > 0.0f && parsed_value <= 14.0f) {
        this->latest_ph_ = parsed_value;
        this->publish_state(this->latest_ph_);
      } else {
        ESP_LOGW(TAG, "Invalid EZO pH response: %s", response.c_str());
      }
      this->waiting_for_response_ = false;
    } else if (millis() - this->request_sent_ms_ > 2000) {
      // Timeout waiting for a response, try again on the next update.
      this->waiting_for_response_ = false;
    }
  }

  if (!this->waiting_for_response_) {
    Wire.beginTransmission(this->address_);
    Wire.write('R');
    Wire.endTransmission();
    this->request_sent_ms_ = millis();
    this->waiting_for_response_ = true;
  }
}

float AtlasEzoPhSensor::get_latest_ph() const {
  return this->latest_ph_;
}

std::string AtlasEzoPhSensor::read_response() {
  std::string response;
  Wire.requestFrom(this->address_, (uint8_t)32);
  while (Wire.available()) {
    const int c = Wire.read();
    if (c == -1) {
      break;
    }
    if (c == '\r' || c == '\n') {
      break;
    }
    response.push_back(static_cast<char>(c));
  }
  return response;
}

}  // namespace pool_controller
