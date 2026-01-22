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
#include <EEPROM.h> // Include EEPROM library

// Initialize sensor (using default pins for ESP32/Arduino auto-detect)
// Adjust pin/voltage/resolution if needed: SCT013 sensor(A0, 5.0, 1024);
SCT013 sensor(A0); // ESP32 Example (Pin 34)

// EEPROM Address for calibration factor
const int EEPROM_ADDR = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial

  // Default calibration (Ratio 2000 / Burden 18 = 111.1)
  sensor.begin(2000, 33); 

  // --- Load from EEPROM ---
  #if defined(ESP32) || defined(ESP8266)
    EEPROM.begin(512); // Initialize EEPROM for ESP
  #endif

  double savedFactor;
  EEPROM.get(EEPROM_ADDR, savedFactor);

  // Simple sanity check: if the value is reasonable (e.g., not NaN or 0 or -1)
  if (!isnan(savedFactor) && savedFactor > 0.1 && savedFactor < 10000.0) {
    sensor.setCalibrationFactor(savedFactor);
    Serial.print("Loaded stored calibration factor: ");
    Serial.println(savedFactor);
  } else {
    Serial.println("No valid calibration found in EEPROM. Using default.");
  }

  Serial.println("=== SCT013 Calibration Utility ===");
  Serial.println("COMMANDS:");
  Serial.println("  't' -> Tare (Zero the sensor)");
  Serial.println("  'r' -> Resistor (Set burden resistor & Reset)");
  Serial.println("  'c' -> Calibrate (Enter known Amps)");
  Serial.println("==================================");
  delay(2000);
}

void loop() {
  // 1. Read Current (Time-based: 10 wavelengths)
  double rawAmps = sensor.readAmps(); 
  double currentFactor = sensor.getCalibrationFactor();

  // --- SMOOTHING FILTER ---
  // A simple "Running Average" to make the numbers steady like a multimeter.
  // Formula: Val = (Old * 0.9) + (New * 0.1)
  static double smoothedAmps = -1.0;
  if (smoothedAmps < 0) smoothedAmps = rawAmps; // First run init
  else smoothedAmps = (smoothedAmps * 0.9) + (rawAmps * 0.1);

  // 2. Print status
  Serial.print("Measured: ");
  Serial.print(smoothedAmps, 3);
  Serial.print(" A  |  Current Factor: ");
  Serial.println(currentFactor, 4);

  // 3. User Input (Non-blocking check)
  if (Serial.available()) {
    char cmd = Serial.read(); 

    // --- CASE T: TARE ---
    if (cmd == 't') {
        sensor.tareNoDelay();
        Serial.println("\n>>> Taring started... (Fast converging to 0) <<<");
        // Clear anything else in buffer
        while(Serial.available()) Serial.read();
    } 
    
    // --- CASE C: CALIBRATE ---
    else if (cmd == 'c') {
        // Clear buffer immediately (remove leftover \n or \r from the 'c' command)
        while(Serial.available()) Serial.read();

        Serial.println("\n--- CALIBRATION MODE ---");
        Serial.println("Enter the REAL Amps shown on your multimeter:");
        
        // Wait for user input (Blocking is fine for a cal sketch)
        // Wait until at least 1 byte is available
        while (Serial.available() == 0) { delay(10); }

        float realAmps = Serial.parseFloat();
        
        // Clear buffer again (remove \n from the number)
        while(Serial.available()) Serial.read();

        if (realAmps > 0.0) {
          double newFactor = currentFactor * (realAmps / measuredAmps);
          
          Serial.println("------------------------------------------------");
          Serial.print(">>> REAL Amps: "); Serial.println(realAmps, 3);
          Serial.print(">>> MEASURED : "); Serial.println(measuredAmps, 3);
          Serial.print(">>> NEW FACTOR: "); Serial.println(newFactor, 5);
          Serial.println("------------------------------------------------");
          Serial.println("Saving to EEPROM...");
          
          // Save to EEPROM
          EEPROM.put(EEPROM_ADDR, newFactor);
          #if defined(ESP32) || defined(ESP8266)
            EEPROM.commit();
          #endif
          
          Serial.println("Saved! Restarting with new factor...");
          Serial.println("------------------------------------------------\n");
          
          // Apply it immediately to test
          sensor.setCalibrationFactor(newFactor);
        } else {
            Serial.println("Invalid value. Cancellation.");
        }
    }
    
    // --- CASE R: RESET / RESISTOR ---
    else if (cmd == 'r') {
        // Clear buffer
        while(Serial.available()) Serial.read();

        Serial.println("\n--- RESET CALIBRATION ---");
        Serial.println("Enter the Burden Resistor value (Ohms) used in your circuit:");
        Serial.println("(Default: 18 for Arduino modules, 62 or 33 custom)");
        
        while (Serial.available() == 0) { delay(10); }

        float resistor = Serial.parseFloat();
        while(Serial.available()) Serial.read();

        if (resistor > 0.0) {
            // Recalculate based on theoretical ratio (2000 turns)
            double newFactor = 2000.0 / resistor;
            
            Serial.println("------------------------------------------------");
            Serial.print(">>> Resistor: "); Serial.print(resistor); Serial.println(" Ohms");
            Serial.print(">>> New Theoretical Factor: "); Serial.println(newFactor, 5);
            Serial.println("------------------------------------------------");
            Serial.println("Saving to EEPROM...");

            EEPROM.put(EEPROM_ADDR, newFactor);
            #if defined(ESP32) || defined(ESP8266)
              EEPROM.commit();
            #endif
            
            Serial.println("Saved! Restarting with new resistor value...");
            sensor.setCalibrationFactor(newFactor);
        } else {
            Serial.println("Invalid resistor value.");
        }
    }
  }

  delay(500);
}
