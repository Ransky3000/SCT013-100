#include "SCT013.h"

SCT013::SCT013(int pin, float voltageReference, int adcResolution) {
    _pin = pin;
    _voltageReference = voltageReference;
    _adcResolution = adcResolution;
    _calibration = 1.0; // Default, must be set by user or defaults
    _offsetI = (1 << _adcResolution) >> 1; // Initial guess: Half of ADC range
    
    // Non-blocking init
    _sumI = 0;
    _sampleCount = 0;
    _samplesToAverage = 1000; // Default sample size
    _lastAmps = 0.0;
    _lastAmps = 0.0;
}

// Auto-Config Constructor
SCT013::SCT013(int pin) {
    _pin = pin;

    #if defined(ESP32)
        _voltageReference = 3.3;
        _adcResolution = 12;
    #else
        // Assume AVR / Standard Arduino (Uno, Mega, Nano)
        _voltageReference = 5.0;
        _adcResolution = 10;
    #endif

    // Same initialization
    _calibration = 1.0; 
    _offsetI = (1 << _adcResolution) >> 1;
    
    _sumI = 0;
    _sampleCount = 0;
    _samplesToAverage = 1000;
    _lastAmps = 0.0;
}

void SCT013::begin(float turnsRatio, float burdenResistor) {
    pinMode(_pin, INPUT);
    setCalibration(turnsRatio, burdenResistor);
}

void SCT013::setCalibration(float turnsRatio, float burdenResistor) {
    // Calibration Factor = Turns Ratio / Burden Resistance
    // Example: 2000 turns / 18 ohms = 111.1
    _calibration = turnsRatio / burdenResistor;
}

double SCT013::readAmps(int samples) {
    double sumI = 0;
    int sampleI;

    for (int i = 0; i < samples; i++) {
        sampleI = analogRead(_pin);

        // Digital Low Pass Filter
        // Removes DC offset dynamically
        // 1024 is the filter constant (time constant) from EmonLib
        _offsetI = (_offsetI + (sampleI - _offsetI) / 1024);
        _filteredI = sampleI - _offsetI;

        // Root-Mean-Square (RMS) Method
        // 1. Square the filtered current
        // 2. Accumulate
        double sqI = _filteredI * _filteredI;
        sumI += sqI;
    }

    // 3. Calculate RMS of ADC counts
    double I_RMS_ADC = sqrt(sumI / samples);

    // 4. Convert ADC counts to Voltage
    // voltage = (adc_rms / max_adc) * v_ref
    double max_adc = (double)(1 << _adcResolution);
    double I_RMS_Volts = (I_RMS_ADC / max_adc) * _voltageReference;

    // 5. Convert Voltage to Amps using Calibration Factor
    return I_RMS_Volts * _calibration;
}

float SCT013::getDCoffset() {
    return (float)_offsetI;
}

bool SCT013::update() {
    int sampleI = analogRead(_pin);

    // Digital Low Pass Filter
    _offsetI = (_offsetI + (sampleI - _offsetI) / 1024);
    _filteredI = sampleI - _offsetI;

    // Accumulate Squares
    double sqI = _filteredI * _filteredI;
    _sumI += sqI;
    _sampleCount++;

    if (_sampleCount >= _samplesToAverage) {
        // Calculate RMS
        double I_RMS_ADC = sqrt(_sumI / _sampleCount);

        // Convert to Voltage
        double max_adc = (double)(1 << _adcResolution);
        double I_RMS_Volts = (I_RMS_ADC / max_adc) * _voltageReference;

        // Convert to Amps
        _lastAmps = I_RMS_Volts * _calibration;

        // Reset
        _sumI = 0;
        _sampleCount = 0;
        return true;
    }

    return false;
}

double SCT013::getLastAmps() {
    return _lastAmps;
}
