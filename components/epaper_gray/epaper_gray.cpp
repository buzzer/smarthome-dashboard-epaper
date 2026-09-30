// SPDX-License-Identifier: GPL-3.0-only
// Graustufen-Kurven aus GxEPD2_4G (Jean-Marc Zingg), siehe NOTICE.md
#include "epaper_gray.h"
#include "esphome/core/application.h"
#include "esphome/core/log.h"

namespace esphome {
namespace epaper_gray {

static const char *const TAG = "epaper_gray";

void EPaperGray::setup() {
  waveshare_epaper::WaveshareEPaper7P5InV2P::setup();
  RAMAllocator<uint8_t> allocator;
  this->plane_old_ = allocator.allocate(this->get_buffer_length_());
  if (this->plane_old_ == nullptr) {
    ESP_LOGE(TAG, "Kein Speicher für die zweite Bildebene, nur Schwarzweiß möglich");
    this->mode_ = BW;
    return;
  }
  memset(this->plane_old_, 0xFF, this->get_buffer_length_());
}

uint8_t EPaperGray::level_(Color c) {
  const uint8_t v = std::max(std::max(c.r, c.g), std::max(c.b, c.w));
  return (v + 42) / 85;  // 0..3
}

void EPaperGray::fill(Color color) {
  const uint8_t lv = level_(color);
  const uint32_t len = this->get_buffer_length_();
  memset(this->buffer_, lv >= 2 ? 0x00 : 0xFF, len);
  if (this->plane_old_ != nullptr) memset(this->plane_old_, (lv == 1 || lv == 3) ? 0x00 : 0xFF, len);
}

void HOT EPaperGray::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (x >= this->get_width_internal() || y >= this->get_height_internal() || x < 0 || y < 0)
    return;
  const uint32_t pos = (x + y * this->get_width_controller()) / 8u;
  const uint8_t mask = 0x80 >> (x & 0x07);
  const uint8_t lv = level_(color);
  if (lv >= 2)
    this->buffer_[pos] &= ~mask;  // "neu": dunkelgrau, schwarz
  else
    this->buffer_[pos] |= mask;
  if (this->plane_old_ != nullptr) {
    if (lv == 1 || lv == 3)
      this->plane_old_[pos] &= ~mask;  // "alt": hellgrau, schwarz
    else
      this->plane_old_[pos] |= mask;
  }
}

void EPaperGray::display() {
  if (this->mode_ == BW || this->plane_old_ == nullptr) {
    if (this->gray_active_) {  // von Graustufen zurück: Controller neu einstellen
      this->initialize();
      this->gray_active_ = false;
    }
    waveshare_epaper::WaveshareEPaper7P5InV2P::display();
    return;
  }
  if (this->mode_ == GRAY_LUT)
    this->display_gray_lut_();
  else
    this->display_gray_otp_();
}

void EPaperGray::hw_reset_() {  // wie im Original (dort privat): high 20 ms, low 2 ms, high 20 ms
  if (this->reset_pin_ == nullptr) return;
  this->reset_pin_->digital_write(true);
  delay(20);  // NOLINT
  this->reset_pin_->digital_write(false);
  delay(2);  // NOLINT
  this->reset_pin_->digital_write(true);
  delay(20);  // NOLINT
}

void EPaperGray::send_lut_(uint8_t cmd, const uint8_t *lut, size_t len) {
  this->command(cmd);
  for (size_t i = 0; i < 42; i++) this->data(i < len ? lut[i] : 0x00);
}

void HOT EPaperGray::display_gray_otp_() {
  const uint32_t len = this->get_buffer_length_();
  ESP_LOGI(TAG, "Bildaufbau mit 4 Graustufen (Werkskurve)");

  // Initialisierung wie Waveshare EPD_7IN5_V2_Init_4Gray()
  this->hw_reset_();
  this->command(0x00);  // Panel Setting: Kurven aus dem Panel-Speicher (OTP)
  this->data(0x1F);
  this->command(0x50);  // VCOM and Data Interval
  this->data(0x10);
  this->data(0x07);
  this->command(0x04);  // Power On
  delay(100);           // NOLINT
  this->wait_until_idle_();
  this->command(0x06);  // Booster Soft Start
  this->data(0x27);
  this->data(0x27);
  this->data(0x18);
  this->data(0x17);
  this->command(0xE0);  // Cascade Setting: Temperaturwert aus 0xE5 verwenden
  this->data(0x02);
  this->command(0xE5);  // Temperaturwert 0x5F wählt die Graustufen-Kurve
  this->data(0x5F);
  this->gray_active_ = true;

  // Bit "alt" (hellgrau, schwarz) und Bit "neu" (dunkelgrau, schwarz); Puffer: 0 = Tinte -> invertieren
  this->command(0x10);
  this->start_data_();
  for (uint32_t i = 0; i < len; i++) {
    this->write_byte(~this->plane_old_[i]);
    if ((i & 0x3FFF) == 0) App.feed_wdt();
  }
  this->end_data_();
  this->command(0x13);
  this->start_data_();
  for (uint32_t i = 0; i < len; i++) {
    this->write_byte(~this->buffer_[i]);
    if ((i & 0x3FFF) == 0) App.feed_wdt();
  }
  this->end_data_();

  this->command(0x12);  // Refresh
  delay(100);           // NOLINT
  this->wait_until_idle_();
  this->command(0x02);  // Power Off
  this->wait_until_idle_();
}

void EPaperGray::dump_config() {
  waveshare_epaper::WaveshareEPaper7P5InV2P::dump_config();
  ESP_LOGCONFIG(TAG, "  Modus: %u, zweite Ebene: %s", this->mode_, YESNO(this->plane_old_ != nullptr));
}

// Graustufen-Kurven für 7,5" 800x480 UC8179 (GDEY075T7) aus GxEPD2_4G, Fork nicoh88 (GPL-3.0)
static const uint8_t LUT20_VCOM[] = {0x00, 0x0A, 0x00, 0x00, 0x00, 0x01, 0x60, 0x14, 0x14, 0x00, 0x00, 0x01,
                                     0x00, 0x14, 0x00, 0x00, 0x00, 0x01, 0x00, 0x13, 0x0A, 0x01, 0x00, 0x01};
static const uint8_t LUT21_WW[] = {0x40, 0x0A, 0x00, 0x00, 0x00, 0x01, 0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
                                   0x10, 0x14, 0x0A, 0x00, 0x00, 0x01, 0xA0, 0x13, 0x01, 0x00, 0x00, 0x01};
static const uint8_t LUT22_BW[] = {0x40, 0x0A, 0x00, 0x00, 0x00, 0x01, 0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
                                   0x00, 0x14, 0x0A, 0x00, 0x00, 0x01, 0x99, 0x0C, 0x01, 0x03, 0x04, 0x01};
// Hellgrau: letzte Phase Richtung Schwarz 2 statt 1 Frame (Original zu hell für kleine Schrift)
static const uint8_t LUT23_WB[] = {0x40, 0x0A, 0x00, 0x00, 0x00, 0x01, 0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
                                   0x00, 0x14, 0x0A, 0x00, 0x00, 0x01, 0x99, 0x0B, 0x04, 0x04, 0x02, 0x01};
static const uint8_t LUT24_BB[] = {0x80, 0x0A, 0x00, 0x00, 0x00, 0x01, 0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
                                   0x20, 0x14, 0x0A, 0x00, 0x00, 0x01, 0x50, 0x13, 0x01, 0x00, 0x00, 0x01};
static const uint8_t LUT25_BD[] = {0x00, 30, 5, 30, 5, 1};

void HOT EPaperGray::display_gray_lut_() {
  const uint32_t len = this->get_buffer_length_();
  ESP_LOGI(TAG, "Bildaufbau mit 4 Graustufen (eigene Kurven)");

  // _InitDisplay() aus GxEPD2_750_T7Y
  this->hw_reset_();
  this->command(0x01);  // Power Setting
  this->data(0x07);
  this->data(0x17);
  this->data(0x3A);
  this->data(0x3A);
  this->data(0x03);
  this->command(0x00);
  this->data(0x1F);
  this->command(0x06);  // Booster Soft Start
  this->data(0x17);
  this->data(0x17);
  this->data(0x28);
  this->data(0x17);
  this->command(0x61);  // Auflösung 800 x 480
  this->data(0x03);
  this->data(0x20);
  this->data(0x01);
  this->data(0xE0);
  this->command(0x15);
  this->data(0x00);
  this->command(0x50);
  this->data(0x10);
  this->data(0x07);
  this->command(0x60);  // TCON
  this->data(0x22);
  // _Init_4G(): Kurven aus den Registern
  this->command(0x00);
  this->data(0x3F);
  this->command(0x50);
  this->data(0x31);
  this->data(0x07);
  this->send_lut_(0x20, LUT20_VCOM, sizeof(LUT20_VCOM));
  this->send_lut_(0x21, LUT21_WW, sizeof(LUT21_WW));
  this->send_lut_(0x22, LUT22_BW, sizeof(LUT22_BW));
  this->send_lut_(0x23, LUT23_WB, sizeof(LUT23_WB));
  this->send_lut_(0x24, LUT24_BB, sizeof(LUT24_BB));
  this->send_lut_(0x25, LUT25_BD, sizeof(LUT25_BD));
  this->command(0x04);  // Power On
  delay(100);           // NOLINT
  this->wait_until_idle_();
  this->gray_active_ = true;

  // Bildspeicher wie GxEPD2: 0x10 = 1 bei weiß/hellgrau, 0x13 = 1 bei weiß/dunkelgrau.
  // Das entspricht direkt buffer_ bzw. plane_old_ (dort 1 = keine Tinte).
  this->command(0x91);  // partial in, Fenster = ganzer Bildschirm
  this->command(0x90);
  this->data(0x00);
  this->data(0x00);
  this->data(0x03);
  this->data(0x1F);
  this->data(0x00);
  this->data(0x00);
  this->data(0x01);
  this->data(0xDF);
  this->data(0x01);
  this->command(0x10);
  this->start_data_();
  for (uint32_t i = 0; i < len; i++) {
    this->write_byte(this->buffer_[i]);
    if ((i & 0x3FFF) == 0) App.feed_wdt();
  }
  this->end_data_();
  this->command(0x13);
  this->start_data_();
  for (uint32_t i = 0; i < len; i++) {
    this->write_byte(this->plane_old_[i]);
    if ((i & 0x3FFF) == 0) App.feed_wdt();
  }
  this->end_data_();
  this->command(0x92);  // partial out

  this->command(0x12);  // Refresh
  delay(100);           // NOLINT
  this->wait_until_idle_();
  this->command(0x02);  // Power Off
  this->wait_until_idle_();
}

}  // namespace epaper_gray
}  // namespace esphome
