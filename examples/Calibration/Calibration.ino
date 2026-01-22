/*
  SCT013-100 Calibration Utility
  
  This sketch helps you find the precise calibration factor for your specific hardware.
  
  HOW TO USE:
  1. Upload this sketch.
  2. Open Serial Monitor (115200 baud).
  3. The sketch will continually print the current (Amps).
  4. Measure the AC line with a trusted multimeter.
  5. Type the REAL value into the Serial Monitor (e.g., "0.45") and press Send.
  6. The sketch will calculate the NEW calibration factor.
  7. Copy this new factor and use `sensor.setCalibrationFactor(NEW_FACTOR)` in your project.
*/

#include <SCT013.h>

// Initialize sensor (using default pins for ESP32/Arduino auto-detect)
// Adjust pin/voltage/resolution if needed: SCT013 sensor(A0, 5.0, 1024);
SCT013 sensor(34); // ESP32 Example (Pin 34)

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial

  // Default calibration (Ratio 2000 / Burden 18 = 111.1)
  // Or Burden 62 = 32.25
  sensor.begin(2000, 62); 

  Serial.println("=== SCT013 Calibration Utility ===");
  Serial.println("1. Connect a KNOWN load (e.g., a lamp).");
  Serial.println("2. Measure current with a multimeter.");
  Serial.println("3. Type the REAL Amps into this Serial Monitor.");
  Serial.println("==================================");
  delay(2000);
}

void loop() {
  // 1. Read Current
  double measuredAmps = sensor.readAmps(500); // 500 samples
  double currentFactor = sensor.getCalibrationFactor();

  // 2. Print status
  Serial.print("Measured: ");
  Serial.print(measuredAmps, 3);
  Serial.print(" A  |  Current Factor: ");
  Serial.println(currentFactor, 4);

  // 3. User Input (Non-blocking check)
  if (Serial.available()) {
    float realAmps = Serial.parseFloat();
    
    // Clear buffer
    while(Serial.available()) Serial.read();

    if (realAmps > 0.0) {
      // THE MAGIC FORMULA:
      // NewFactor = OldFactor * (Real / Measured)
      
      double newFactor = currentFactor * (realAmps / measuredAmps);
      
      Serial.println("\n------------------------------------------------");
      Serial.print(">>> REAL Amps: "); Serial.println(realAmps, 3);
      Serial.print(">>> MEASURED : "); Serial.println(measuredAmps, 3);
      Serial.print(">>> NEW FACTOR: "); Serial.println(newFactor, 5);
      Serial.println("------------------------------------------------");
      Serial.println("Update your code with: ");
      Serial.print("sensor.setCalibrationFactor(");
      Serial.print(newFactor, 5);
      Serial.println(");");
      Serial.println("------------------------------------------------\n");
      
      // Apply it immediately to test
      sensor.setCalibrationFactor(newFactor);
      delay(2000); // Pause to let user read
    }
  }

  delay(500);
}
