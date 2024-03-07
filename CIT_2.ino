#include <Arduino.h>
#include "Wire.h"
#include "MAX30100_PulseOximeter.h"
#include "HX711.h"

const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;
const int BUTTON1_PIN = 4; // Pin connected to button1 (for pulse oximeter readings)
const int BUTTON2_PIN = 5; // Pin connected to button2 (for weight readings)
const unsigned long REPORTING_PERIOD_MS = 1000; // Reporting period for pulse oximeter readings

HX711 scale;
PulseOximeter pox;
uint32_t tsLastReport = 0;
boolean pulseEnabled = false;  // Flag to track if pulse oximeter readings are enabled
boolean weightEnabled = false; // Flag to track if weight readings are enabled

void onBeatDetected()
{
    Serial.println("Beat!");
}

void setup()
{
    Serial.begin(57600);
    Serial.println("Initializing the scale and pulse oximeter");

    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

    pinMode(BUTTON1_PIN, INPUT_PULLUP); // Configure button1 pin as input with internal pull-up resistor
    pinMode(BUTTON2_PIN, INPUT_PULLUP); // Configure button2 pin as input with internal pull-up resistor

    if (!pox.begin()) {
        Serial.println("Pulse Oximeter FAILED");
        for (;;) ;
    } else {
        Serial.println("Pulse Oximeter SUCCESS");
    }
    scale.set_scale(-400.542);
    scale.tare();
    scale.read();
    scale.get_value(5);
    scale.get_units(5), 1;

    pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
    pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop()
{
    // Check if button1 is pressed to toggle pulse oximeter readings
    if (digitalRead(BUTTON1_PIN) == LOW)
    {
        pulseEnabled = !pulseEnabled;
        weightEnabled = false; // Turn off weight readings
        delay(500); // Debounce delay
    }

    // Check if button2 is pressed to toggle weight readings
    if (digitalRead(BUTTON2_PIN) == LOW)
    {
        weightEnabled = !weightEnabled;
        pulseEnabled = false; // Turn off pulse oximeter readings
        delay(500); // Debounce delay
    }

    if (pulseEnabled)
    {
        // Enable pulse oximeter readings
        pox.update();
        if (millis() - tsLastReport > REPORTING_PERIOD_MS)
        {
            Serial.print("Heart rate:");
            Serial.print(pox.getHeartRate());
            Serial.print(" bpm / SpO2:");
            Serial.print(pox.getSpO2());
            Serial.println("%");
            tsLastReport = millis();
        }
    }
    else if (weightEnabled)
    {
        // Enable weight readings
        Serial.print("Weight: ");
        Serial.print(scale.get_units(10), 5
        );
        Serial.println(" kg");
    }
}
