#include <Wire.h>
#include "MAX30105.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)

// Initialize OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Initialize MAX30105 sensor
MAX30105 particleSensor;

long lastIrValue = 0;
long lastRedValue = 0;
unsigned long lastTime = 0;
unsigned long pulseTime = 0;
int pulseCount = 0;
float SpO2 = 0.0;  // Oxygen level
int irSamples[50] = {0}; // Array to store IR values for averaging
int redSamples[50] = {0}; // Array to store Red values for averaging
int sampleIndex = 0; // Index for the averaging buffer

// Threshold for finger detection (adjust as needed)
long irThreshold = 1000;  // Below this value, we assume no finger
unsigned long steadyTimeThreshold = 3000;  // Time in milliseconds to wait before assuming no finger is detected
unsigned long lastSteadyTime = 0;  // To store the time when the finger became steady

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }
  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();

  // Initialize MAX30105 sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) { // Use default I2C port, 400kHz speed
    Serial.println("MAX30105 was not found. Please check wiring/power.");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.println("MAX30105 not found.");
    display.display();
    while (1);
  }
  Serial.println("Connected");

  // Configure sensor settings
  byte ledBrightness = 0x1F; // Options: 0=Off to 255=50mA
  byte sampleAverage = 8;    // Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 3;          // Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  int sampleRate = 100;      // Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411;      // Options: 69, 118, 215, 411
  int adcRange = 4096;       // Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
}

void loop() {
  long irValue = particleSensor.getIR();   // Get IR value
  long redValue = particleSensor.getRed(); // Get Red value

  // Add the latest values to the sample buffers
  irSamples[sampleIndex] = irValue;
  redSamples[sampleIndex] = redValue;
  
  // Increment sample index
  sampleIndex++;
  if (sampleIndex >= 50) {
    sampleIndex = 0; // Reset index after buffer is full
  }

  // Calculate the average IR and Red values
  long avgIR = 0;
  long avgRed = 0;
  for (int i = 0; i < 50; i++) {
    avgIR += irSamples[i];
    avgRed += redSamples[i];
  }
  avgIR /= 50;  // Average of IR
  avgRed /= 50; // Average of Red

  // If no finger is detected (signal too low), set SpO2 to ambient level (21%)
  if (avgIR < irThreshold || avgRed < irThreshold) {
    SpO2 = 21.0;  // Set to ambient oxygen level when no finger is detected
    lastSteadyTime = millis();  // Reset the time when the signal became steady
  } else {
    // Calculate SpO2 based on the ratio of the AC components
    float ratio = 0;
    if (avgIR > 0 && avgRed > 0) {
      ratio = (float)avgIR / avgRed;
    }

    // Estimate SpO2 using a corrected formula
    if (ratio > 0) {
      SpO2 = 100 - 5 * (ratio - 0.4);  // A simple formula for SpO2 estimation
    }

    // Check if the values have been stable for a while (i.e., no significant fluctuations)
    if (abs(lastIrValue - irValue) < 20 && abs(lastRedValue - redValue) < 20) {
      if (millis() - lastSteadyTime > steadyTimeThreshold) {
        // If the readings are stable for a specified time, assume the finger is removed
        SpO2 = 21.0;  // Set to ambient oxygen level when no finger is detected
      }
    } else {
      lastSteadyTime = millis(); // Reset the steady time when there's fluctuation
    }
  }

  // Update the display every second
  if (millis() - lastTime >= 1000) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);  // Position cursor for SpO2 display
    display.print("Oxygen Level: ");
    display.setTextSize(2);
    display.setCursor(0, 10);  // Position cursor for SpO2 value display
    display.print(SpO2, 1);  // Display SpO2 with one decimal place

    // Draw a progress bar
    int progressWidth = map(SpO2, 21, 100, 0, SCREEN_WIDTH - 10); // Map SpO2 value to progress bar width
    display.fillRect(5, 45, progressWidth, 10, SSD1306_WHITE); // Draw the progress bar
    display.drawRect(5, 45, SCREEN_WIDTH - 10, 10, SSD1306_WHITE); // Draw the outline of the progress bar

    // Show the percentage sign
    display.print("%");

    display.display();  // Display everything on the screen

    lastTime = millis();  // Update lastTime to current time
  }

  // Optional: Print SpO2 value to Serial Monitor for debugging
  Serial.print("SpO2: ");
  Serial.println(SpO2);
  
  // Store current IR and Red values for the next iteration
  lastIrValue = irValue;
  lastRedValue = redValue;
}
