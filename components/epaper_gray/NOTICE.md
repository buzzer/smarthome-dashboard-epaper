# epaper_gray – license and origin

This component is licensed under the GNU General Public License version 3 (see `LICENSE`).

## Third-party code

- **Grayscale waveforms** (`LUT20_VCOM` … `LUT25_BD` in `epaper_gray.cpp`) and the command sequence in
  `display_gray_lut_()` are taken from `GxEPD2_750_T7Y.cpp` of the GxEPD2_4G library by Jean-Marc Zingg,
  fork https://github.com/nicoh88/GxEPD2_4G. The fork has no license file of its own; the underlying
  library GxEPD2 (https://github.com/ZinggJM/GxEPD2) is licensed under GPL-3.0.
  According to the source, the waveforms go back to example code by Good Display (GDEY075T7).
  Modified: `LUT23_WB`, last phase towards black 2 frames instead of 1 (darker light gray).
- **Grayscale with OTP waveform** (`display_gray_otp_()`) follows Waveshare's example code
  (EPD_7in5_V2, 4-gray).
- **Base class** `WaveshareEPaper7P5InV2P` from ESPHome (https://github.com/esphome/esphome);
  the C++ part of ESPHome is licensed under GPL-3.0.
