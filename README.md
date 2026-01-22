# SCT013-100 Library for Arduino & ESP32

A lightweight, robust, and cross-platform library for the **SCT013 Series Non-Invasive Current Sensors**. 

Designed to be **simple** for beginners yet **powerful** for advanced IoT applications (ESP32/Blynk/WiFi).

## Features
*   **Auto-Configuration**: Automatically detects if you are compiling for **Arduino (5V, 10-bit)** or **ESP32 (3.3V, 12-bit)** and adjusts math accordingly.
*   **Precision Timing**: Samples for exactly **10 line cycles** (e.g., 200ms @ 50Hz) to ensure stable RMS readings regardless of loop speed.
*   **Digital Smoothing**: Built-in helper to stabilize jittery readings (`smooth()` method).
*   **Zero/Tare**: Non-blocking `tareNoDelay()` function to remove DC offset on the fly.
*   **Calibration Utility**: Includes a sketch to fine-tune your sensor against a multimeter using Serial commands.
*   **Non-Blocking Mode**: Includes an `update()` method so you can read current *without* stopping your main loop.

## Installation
1.  Download this repository as a `.zip` file.
2.  Open Arduino IDE -> **Sketch** -> **Include Library** -> **Add .ZIP Library...**
3.  Select the downloaded zip.

## Wiring (Recommended)
This library works best with the following circuit topography:
*   **Sensor**: SCT013-100 (or similar) connected to an analog pin.
*   **Burden Resistor**: **18Ω** or **33Ω** (18Ω recommended for ESP32/Arduino cross-compatibility).
*   **Bias Circuit**: Two 10kΩ resistors (Voltage Divider) + 10uF Capacitor.

## Usage

### 1. Blocking Mode (Simple)
Best for basic monitoring where the sensor is the main priority.

```cpp
#include <SCT013.h>

// Connect Output to Analog Pin (A0 for Arduino, 34 for ESP32, etc.)
SCT013 sensor(A0);

void setup() {
  Serial.begin(115200);
  
  // Calibration: 2000 turns, 18 ohm burden resistor
  sensor.begin(2000, 18);
}

void loop() {
  // Read RMS Current (Calculates over 10 line cycles)
  double amps = sensor.readAmps();
  
  Serial.print("Current: ");
  Serial.print(amps);
  Serial.println(" A");
  
  delay(1000);
}
```

### 2. Non-Blocking Mode (Advanced)
Best for IoT projects where you need to keep WiFi/Display active.

```cpp
#include <SCT013.h>

SCT013 sensor(A0);

void setup() {
  Serial.begin(115200);
  sensor.begin(2000, 18);
}

void loop() {
  // Call update() as fast as possible in your loop
  // It returns TRUE when a fresh reading is ready (every ~200ms)
  if (sensor.update()) {
    double amps = sensor.getLastAmps();
    
    Serial.print("Current: ");
    Serial.println(amps);
  }
}
```

## Calibration Utility
This library comes with a powerful **Calibration Sketch** (`examples/Calibration/Calibration.ino`). Upload it to your board to fine-tune your readings via Serial Monitor.

**Supported Commands:**
*   **`t` (Tare)**: Zeros the sensor (removes DC offset).
*   **`c` (Calibrate)**: Enter the *Real Amps* from your multimeter, and the storage automatically calculates the new factor.
*   **`r` (Reset)**: Enter your burden resistor value (e.g., 18 or 33) to reset calibration to theoretical defaults.
*   **`s` (Smooth)**: Adjust the smoothing filter weight (0.1 = Fast, 0.9 = Slow/Stable).

## API Reference

### `SCT013 sensor(pin)`
Constructor. Automatically detects board voltage and ADC resolution.

### `void begin(turns, burdenOhms)`
Initializes the sensor parameters.
*   `turns`: The number of turns using in the CT (e.g., **2000**).
*   `burdenOhms`: The value of your burden resistor (e.g., **18** or **33**).

### `void setFrequency(hz)`
Sets the mains frequency for timing calculations.
*   `hz`: **50** or **60**. (Default: 50).

### `double readAmps()`
**Blocking**. Samples for 10 full cycles and returns the RMS Amps. Recommended for highest accuracy if blocking is okay.

### `bool update()`
**Non-blocking**. Call this in `loop()`. Returns `true` when a new reading is available.

### `double getLastAmps()`
Returns the most recent current reading calculated by `update()`.

### `void tareNoDelay()`
Initiates a fast-convergence filter to re-center the DC offset (Zero point). Call this if your readings drift when no load is connected.

### `double smooth(newVal, oldVal, weight)`
Helper function for Exponential Moving Average (EMA) smoothing.
*   `newVal`: The latest reading.
*   `oldVal`: The previous smoothed value.
*   `weight`: The "heaviness" of the old value (0.0 - 1.0).
    *   **0.9**: Very smooth, slow response.
    *   **0.2**: Fast response, less smoothing.

