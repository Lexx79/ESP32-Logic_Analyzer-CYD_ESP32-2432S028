# Project: Logic Analyzer for ESP32 (CYD)

**Version: 1.2**

## Description

This firmware transforms the **ESP32-2432S028 (Cheap Yellow Display)** board into a simple two-channel logic analyzer with touch controls. The project was iteratively developed to capture and analyze digital signals, particularly for debugging protocols like NEC from IR remotes.

## Hardware

*   **Board:** ESP32-2432S028 "Cheap Yellow Display"
*   **Channel 1 (CH1):** GPIO 22
*   **Channel 2 (CH2):** GPIO 27 (Connector CN1)
    *   *Note: GPIO 35 (Connector P3) can also be used for CH2, but it may be faulty in some cases.*

## Features

*   **Two Channels:** Capture and display two independent digital signals.
*   **Touch Interface:** All control is handled via 4 main buttons and 2 additional shift buttons.
*   **Trigger Modes:**
    *   `AUTO`: Continuous capture without synchronization.
    *   `FALL`: Continuous capture synchronized on the falling edge of CH1 or CH2.
    *   `RISE`: Continuous capture synchronized on the rising edge of CH1 or CH2.
    *   `SINGLE`: Single-shot capture on the falling edge of CH1 or CH2. Ideal for capturing non-periodic signals (e.g., from a remote control).
*   **Timebase Control:** The `T-` and `T+` buttons allow for flexible adjustment of the capture time (from 0 to 4000 µs per sample).
*   **Captured Signal Analysis:**
    *   In `SINGLE` mode, after a capture (when "paused"), the signal can be examined in detail.
    *   **Zooming:** The `T-` and `T+` buttons allow you to "zoom in" and "zoom out" on the signal, and the `T:us` time value on the screen is automatically recalculated.
    *   **Shifting:** Additional `<-` and `->` buttons appear when paused and allow you to shift the viewing area across the entire capture buffer (2048 samples).
*   **Rendering:** Signals are drawn as clear rectangular pulses for ease of analysis.
*   **Splash Screen:** An initial splash screen with the software name and version is displayed on startup.

## How to Use

1.  **Select `TRIG` Mode:** Use the `TRIG` button to select the desired mode. For capturing a signal from a remote, use `SINGLE`.
2.  **Adjust Timebase `T-`/`T+`:** Set the desired base capture time. A value of around 50us works well for IR signals.
3.  **Capture:**
    *   In `SINGLE` mode, press the green `ARM` button. The device will enter `WAITING...` mode. Apply a signal to the CH1 or CH2 input. It will automatically "freeze" after capture.
    *   In continuous modes (`AUTO`, `FALL`, `RISE`), capture is ongoing.
4.  **Analysis (when paused):**
    *   Use `T-`/`T+` to change the zoom level.
    *   Use `<-`/`->` to pan across the waveform.
    *   To take a new measurement (in `SINGLE`) or resume operation (in `AUTO`/`FALL`/`RISE`), press `ARM` / `RUN` again.

---
### Version History

*   **v1.2 (10.01.2026):**
    *   Added an initial splash screen on device startup. The code was reorganized using header files.
*   **v1.1:**
    *   Initial stable version. All core functionality implemented.
