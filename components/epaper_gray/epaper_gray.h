// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "esphome/components/waveshare_epaper/waveshare_epaper.h"

namespace esphome {
namespace epaper_gray {

// Graustufen als Farben: Helligkeit 0 = weiß, 85 = hellgrau, 170 = dunkelgrau, 255 = schwarz (wie COLOR_ON)
static const Color GRAY_LIGHT(85, 85, 85, 85);
static const Color GRAY_DARK(170, 170, 170, 170);

/**
 * Waveshare 7.5" V2 mit 4 Graustufen.
 *
 * buffer_ (geerbt) hält das Bit "neu" (Befehl 0x13): gesetzt bei dunkelgrau und schwarz.
 * plane_old_ hält das Bit "alt" (Befehl 0x10): gesetzt bei hellgrau und schwarz.
 * Beide mit der Polarität des Originals: 0 = Tinte, 1 = weiß. Im Schwarzweiß-Betrieb bleibt der
 * geerbte Ablauf unverändert; hellgrau erscheint dort weiß, dunkelgrau schwarz.
 */
class EPaperGray : public waveshare_epaper::WaveshareEPaper7P5InV2P {
 public:
  void setup() override;
  // 0 = schwarzweiß, 1 = Graustufen mit Werkskurve (Waveshare), 2 = Graustufen mit eigenen Kurven (GxEPD2_4G)
  enum Mode : uint8_t { BW = 0, GRAY_OTP = 1, GRAY_LUT = 2 };
  void set_grayscale(bool gray) { this->mode_ = gray ? GRAY_LUT : BW; }
  void set_mode(uint8_t mode) { this->mode_ = mode; }
  uint8_t get_mode() const { return this->mode_; }

  void fill(Color color) override;
  void display() override;
  void dump_config() override;

 protected:
  void draw_absolute_pixel_internal(int x, int y, Color color) override;
  static uint8_t level_(Color c);  // 0 weiß .. 3 schwarz
  void display_gray_otp_();
  void display_gray_lut_();
  void hw_reset_();
  void send_lut_(uint8_t cmd, const uint8_t *lut, size_t len);

  uint8_t mode_{BW};
  bool gray_active_{false};  // Controller steht auf der Graustufen-Kurve
  uint8_t *plane_old_{nullptr};
};

}  // namespace epaper_gray
}  // namespace esphome
