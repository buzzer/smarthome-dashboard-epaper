// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "esphome/components/waveshare_epaper/waveshare_epaper.h"

namespace esphome {
namespace epaper_gray {

// Gray levels as colors: 0 = white, 85 = light gray, 170 = dark gray, 255 = black (like COLOR_ON)
static const Color GRAY_LIGHT(85, 85, 85, 85);
static const Color GRAY_DARK(170, 170, 170, 170);

/**
 * Waveshare 7.5" V2 with 4 gray levels.
 *
 * buffer_ (inherited) holds the "new" bit (command 0x13): set for dark gray and black.
 * plane_old_ holds the "old" bit (command 0x10): set for light gray and black.
 * Both use the original polarity: 0 = ink, 1 = white. In black-and-white mode the inherited
 * sequence is unchanged; light gray shows as white there, dark gray as black.
 */
class EPaperGray : public waveshare_epaper::WaveshareEPaper7P5InV2P {
 public:
  void setup() override;
  // 0 = black and white, 1 = grayscale with OTP waveform (Waveshare), 2 = grayscale with custom waveforms (GxEPD2_4G)
  enum Mode : uint8_t { BW = 0, GRAY_OTP = 1, GRAY_LUT = 2 };
  void set_grayscale(bool gray) { this->mode_ = gray ? GRAY_LUT : BW; }
  void set_mode(uint8_t mode) { this->mode_ = mode; }
  uint8_t get_mode() const { return this->mode_; }

  void fill(Color color) override;
  void display() override;
  void dump_config() override;

 protected:
  void draw_absolute_pixel_internal(int x, int y, Color color) override;
  static uint8_t level_(Color c);  // 0 white .. 3 black
  void display_gray_otp_();
  void display_gray_lut_();
  void hw_reset_();
  void send_lut_(uint8_t cmd, const uint8_t *lut, size_t len);

  uint8_t mode_{BW};
  bool gray_active_{false};  // controller is set to the grayscale waveform
  uint8_t *plane_old_{nullptr};
};

}  // namespace epaper_gray
}  // namespace esphome
