#include <SCT013.h>

// --- HARDWARE CONFIGURATION ---
#if defined(ESP32)
  #define ADC_PIN 34
#else 
  #define ADC_PIN A0
#endif

// Initialize Scanner (Auto-detects Volts & Resolution)
SCT013 sensor(ADC_PIN);

void setup() {
  Serial.begin(115200);
  
  // Initialize w/ Calibration: 2000 turns, 18 ohm burden
  sensor.begin(2000, 18);

  Serial.println("SCT013 Current Sensor Ready");
  
  // Debug: Check DC Offset
  Serial.print("DC Offset Target: ");
#if defined(ESP32)
  Serial.println(2048);
#else
  Serial.println(512);
#endif
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
