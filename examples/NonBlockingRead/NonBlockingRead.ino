#include <SCT013.h>

// --- HARDWARE CONFIGURATION ---
// ESP32: 3.3V, 12-bit ADC (0-4095), Pin 34 (example)
// Arduino: 5.0V, 10-bit ADC (0-1023), Pin A0

#if defined(ESP32)
  #define ADC_PIN 34
  #define V_REF 3.3
  #define ADC_RES 12
#else 
  // Arduino Uno/Mega
  #define ADC_PIN A0
  #define V_REF 5.0
  #define ADC_RES 10
#endif

// Burden Resistor (Ohms)
// USE 18 OHMS for ESP32/Arduino compatibility!
#define BURDEN_RESISTOR 18.0 
#define CT_TURNS 2000.0

SCT013 sensor(ADC_PIN, V_REF, ADC_RES);

void setup() {
  Serial.begin(115200);
  
  sensor.begin();
  sensor.setCalibration(CT_TURNS, BURDEN_RESISTOR);

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
