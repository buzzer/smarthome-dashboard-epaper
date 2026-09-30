# SPDX-License-Identifier: GPL-3.0-only
"""Waveshare 7.5" V2 (new panels from 10/2023) with 4 gray levels.

Inherits the black-and-white sequence from waveshare_epaper (model 7.50inV2p) and adds a second image
plane plus two grayscale sequences: Waveshare's OTP waveform (temperature value 0x5F) and custom
register waveforms from GxEPD2_4G.
"""

CODEOWNERS = []
