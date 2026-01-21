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

// CT Turns Ratio (2000 for SCT013-000)
#define CT_TURNS 2000.0

// Initialize Scanner
SCT013 sensor(ADC_PIN, V_REF, ADC_RES);

void setup() {
  Serial.begin(115200);
  
  // Initialize Sensor
  sensor.begin();

  // Set calibration (Turns / Burden)
  // 2000 / 18 = 111.1
  sensor.setCalibration(CT_TURNS, BURDEN_RESISTOR);

  Serial.println("SCT013 Current Sensor Ready");
  Serial.print("DC Offset Target: ");
  Serial.println((1 << ADC_RES) / 2); // Expecting ~2048 (ESP32) or ~512 (Arduino)
}

void loop() {
  // Read RMS Current (take 1000 samples)
  // More samples = smoother reading but slower loop
  double amps = sensor.readAmps(1000);

  Serial.print("Current: ");
  Serial.print(amps);
  Serial.print(" A");

  // Debug: Check DC Offset
  // If this is 0 or MaxADC, check your hardware!
  Serial.print(" | DC Offset: ");
  Serial.print(sensor.getDCoffset());

  Serial.println();
  
  delay(500); 
}
