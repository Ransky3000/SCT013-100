#include <SCT013.h>

// --- HARDWARE CONFIGURATION ---
// ESP32 or Arduino Uno/Mega? 
// The library now auto-detects this! Just pass the pin.

#if defined(ESP32)
  #define ADC_PIN 34
#else 
  #define ADC_PIN A0
#endif

// Initialize Scanner (Auto-detects Volts & Resolution)
SCT013 sensor(ADC_PIN);

void setup() {
  Serial.begin(115200);
  
  // Initialize with Calibration (Turns, Burden Ohms)
  // Default SCT013 turns = 2000
  // Recommended Burden = 18 Ohms
  sensor.begin(2000, 18);
  
  // Set Frequency (Default 50Hz, change to 60Hz if needed)
  sensor.setFrequency(60);

  Serial.println("SCT013 Non-Blocking Example");
}

unsigned long lastPrintTime = 0;

void loop() {
  // Call update() as fast as possible
  // It returns TRUE when a fresh calculation is ready (every ~1000 samples)
  if (sensor.update()) {
    
    // Example: Only print every 500ms to avoid spamming Serial
    if (millis() - lastPrintTime > 500) {
      lastPrintTime = millis();
      
      double amps = sensor.getLastAmps();
      
      Serial.print("Current: ");
      Serial.print(amps);
      Serial.println(" A");
    }
  }

  // Do other things here (blink LEDs, WiFi, etc)
  // They won't be blocked!
}
