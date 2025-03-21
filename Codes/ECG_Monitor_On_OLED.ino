#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width
#define SCREEN_HEIGHT 64  // OLED display height
#define OLED_RESET    -1  // Reset pin (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Replace A0 with your correct STM32 analog pin
#define ECG_PIN PA0  // Change this to the correct ADC pin for your STM32 board

// Variables to hold ECG data
int ecgData[SCREEN_WIDTH];  // Array to store the ECG values (to display graph)
int baseline = SCREEN_HEIGHT / 5;  // Shift the baseline higher (upper part of the screen for ECG graph)
int zoomFactor = 10;  // Increased zoom factor for more zoomed-in display

void setup() {
  Serial.begin(115200);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3C for 128x64 OLED
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }
  display.clearDisplay();
  display.display();
  delay(2000);  // Pause for 2 seconds

  // Set text properties for title
  display.setTextSize(2);  // Larger text for the title
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);  // Move text to the top
  display.print("ECG Signal");
  display.display();
  delay(1000);  // Pause before graph display

  // Draw a simple heart graphic or something relevant
  display.setTextSize(1);
  display.setCursor(50, 10);
  display.print("<3");
  display.display();
  delay(500);  // Pause before graph display

  // Initialize ECG data with baseline
  for (int i = 0; i < SCREEN_WIDTH; i++) {
    ecgData[i] = baseline;  // Start with a baseline at the middle of the screen
  }
}

void loop() {
  // Shift the ECG data left (to simulate moving graph)
  for (int i = 0; i < SCREEN_WIDTH - 1; i++) {
    ecgData[i] = ecgData[i + 1];
  }

  // Read the ECG sensor value from the analog pin (ADC pin)
  int ecgValue = analogRead(ECG_PIN);  // Read the ECG signal

  // Check if the sensor detects a valid signal
  if (ecgValue > 1000) {  // A threshold to detect signal, adjust as needed
    // Simulate the effect of a full-wave rectifier (take the absolute value)
    int rectifiedValue = abs(ecgValue - 512);  // Subtract 512 to center the signal, then take the absolute value

    // Zoom in the rectified ECG value and map it to a smaller vertical range
    int scaledEcgValue = map(rectifiedValue, 0, 1023, baseline - zoomFactor, baseline + zoomFactor);

    // Clamp the lower signal to upper part (force negative parts to stay above baseline)
    if (scaledEcgValue < baseline) {
      scaledEcgValue = baseline;
    }

    // Add the new value to the right side of the array
    ecgData[SCREEN_WIDTH - 1] = scaledEcgValue;
  } else {
    // If no signal detected, keep the graph flat at baseline
    ecgData[SCREEN_WIDTH - 1] = baseline;
  }

  // Clear the display to redraw the graph
  display.clearDisplay();

  // Draw the ECG graph
  for (int i = 1; i < SCREEN_WIDTH; i++) {
    // Draw a line connecting the ECG data points
    display.drawLine(i - 1, ecgData[i - 1], i, ecgData[i], SSD1306_WHITE);
  }

  // Add labels and text on top of the graph (adjusted to be at the top)
  display.setTextSize(1);  // Smaller text for bottom labels
  display.setCursor(0, SCREEN_HEIGHT - 8);  // Place text at bottom
  display.setTextColor(SSD1306_WHITE);
  display.print("ECG Signal");

  // Display the ECG graph
  display.display();

  // Add a small delay for smooth animation
  delay(20);
}
