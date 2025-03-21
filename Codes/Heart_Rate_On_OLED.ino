#include <Wire.h>
#include "MAX30105.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)

// Initialize OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Initialize MAX30105 sensor
MAX30105 particleSensor;

// Define the heart bitmap (16x16 pixels)
const uint8_t heartBitmap[] PROGMEM = {
  0b00001100, 0b00110000,
  0b00011110, 0b01111000,
  0b00111111, 0b11111100,
  0b01111111, 0b11111110,
  0b01111111, 0b11111110,
  0b01111111, 0b11111110,
  0b00111111, 0b11111100,
  0b00011111, 0b11111000,
  0b00001111, 0b11110000,
  0b00000111, 0b11100000,
  0b00000011, 0b11000000,
  0b00000001, 0b10000000,
  0b00000000, 0b00000000,
};

#define HEART_WIDTH 16
#define HEART_HEIGHT 13

unsigned long lastBeatTime = 0;
float beatsPerMinute = 0;
float bpmBuffer[5] = {0};  // Buffer for sliding window average (5 values)
int bpmIndex = 0;          // Index for circular buffer
long previousIR = 0;
int peakThreshold = 5000;  // Initial threshold (can be dynamically adjusted)
float smoothedThreshold = 5000;

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
  long irValue = particleSensor.getIR(); // Get IR value
  unsigned long currentTime = millis();

  // Dynamic threshold adjustment using exponential smoothing
  smoothedThreshold = (0.9 * smoothedThreshold) + (0.1 * irValue);

  // Inter-peak interval method: Calculate BPM based on time difference between peaks
  if (irValue > smoothedThreshold && previousIR <= smoothedThreshold) {
    unsigned long timeSinceLastBeat = currentTime - lastBeatTime;
    lastBeatTime = currentTime;

    // Prevent unrealistic BPM values
    if (timeSinceLastBeat > 300 && timeSinceLastBeat < 2000) {
      beatsPerMinute = 60000.0 / timeSinceLastBeat;

      // Sliding window average for smoothing
      bpmBuffer[bpmIndex] = beatsPerMinute;
      bpmIndex = (bpmIndex + 1) % 5; // Circular buffer
    }
  }

  // Calculate average BPM from the sliding window
  float bpmSum = 0;
  int validCount = 0;
  for (int i = 0; i < 5; i++) {
    if (bpmBuffer[i] > 0) { // Ignore uninitialized buffer values
      bpmSum += bpmBuffer[i];
      validCount++;
    }
  }
  float averagedBPM = (validCount > 0) ? (bpmSum / validCount) : 0;
  averagedBPM = floor(averagedBPM);

  // Display the heart bitmap and heart rate (BPM) on OLED
  display.clearDisplay();

  // Draw the heart icon at the top-left corner
  display.drawBitmap(0, 0, heartBitmap, HEART_WIDTH, HEART_HEIGHT, SSD1306_WHITE);

  // Display the heart rate (HR) text next to the heart icon
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 5);
  display.print("HR: ");
  display.println(int(averagedBPM));

  // Display the raw IR value for debugging
  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print("IR: ");
  display.println(irValue);

  display.display();

  // Print to Serial Monitor for debugging
  Serial.print("IR: ");
  Serial.println(irValue);
  Serial.print("BPM: ");
  Serial.println(averagedBPM);

  previousIR = irValue; // Update the previous IR value
  delay(100); // Update every 100ms
}
