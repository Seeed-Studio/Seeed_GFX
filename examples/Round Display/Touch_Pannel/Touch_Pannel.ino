#include <Arduino.h>
#include <TFT_eSPI.h> 
#include <SPI.h>
#include <Wire.h> // Make sure Wire.h is included for I2C communication

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

#define CHSC6X_I2C_ID 0x2e
#define CHSC6X_MAX_POINTS_NUM 1
#define CHSC6X_READ_POINT_LEN 5
#define TOUCH_INT D7

// TFT_eSPI instance
TFT_eSPI tft = TFT_eSPI();   

/**
 * @brief Initializes the touch controller hardware.
 */
void round_display_touch_init(void)
{
  Serial.println("Initializing touch controller...");
  pinMode(TOUCH_INT, INPUT_PULLUP);
  Serial.printf("Set pin %d to INPUT_PULLUP for touch interrupt.\n", TOUCH_INT);
  
  Wire.begin(); // Turn on the I2C bus for the touch driver
  Serial.println("I2C bus started for touch driver.");
}

/**
 * @brief Checks if the screen is currently being pressed.
 * @return true if the screen is pressed, false otherwise.
 */
bool chsc6x_is_pressed(void)
{
  // The interrupt pin is active LOW.
  if(digitalRead(TOUCH_INT) != LOW) {
    // A small delay for debouncing in case of electrical noise.
    delay(1);
    if(digitalRead(TOUCH_INT) != LOW) {
      return false; // Confirmed not pressed.
    }
  }
  // If we reach here, the pin is LOW, indicating a touch event.
  Serial.println("Touch event detected (INT pin is LOW).");
  return true;
}

/**
 * @brief Reads the touch coordinates from the CHSC6X controller via I2C.
 *        It also handles the coordinate transformation based on screen rotation.
 * @param x Reference to a variable that will store the final X coordinate.
 * @param y Reference to a variable that will store the final Y coordinate.
 * @return true if a valid touch point was read and processed, false otherwise.
 */
bool chsc6x_get_xy(int32_t &x, int32_t &y)
{
  uint8_t temp[CHSC6X_READ_POINT_LEN] = {0};
  
  Serial.printf("Requesting %d bytes from I2C device 0x%X...\n", CHSC6X_READ_POINT_LEN, CHSC6X_I2C_ID);
  uint8_t read_len = Wire.requestFrom(CHSC6X_I2C_ID, CHSC6X_READ_POINT_LEN);

  if (read_len == CHSC6X_READ_POINT_LEN) {
    Serial.printf("Successfully received %d bytes.\n", read_len);
    Wire.readBytes(temp, read_len);

    // Log the raw data received from the touch controller for debugging
    Serial.print("Raw touch data: ");
    for(int i=0; i<CHSC6X_READ_POINT_LEN; i++) {
      Serial.printf("0x%02X ", temp[i]);
    }
    Serial.println();

    // Check if the touch data is valid (temp[0] == 0x01 indicates a valid single touch point)
    if (temp[0] == 0x01) {
      int32_t raw_x = temp[2];
      int32_t raw_y = temp[4];
      Serial.printf("Valid touch point detected. Raw coordinates: X=%d, Y=%d\n", raw_x, raw_y);

      // --- Coordinate Rotation Logic ---
      // Get the current screen rotation from the TFT library
      uint8_t rotation = tft.getRotation();
      Serial.printf("Applying screen rotation logic for rotation value: %d\n", rotation);

      // Apply transformation based on rotation
      switch(rotation) {
        case 0: // 0 degrees (Portrait)
          x = raw_x;
          y = raw_y;
          break;
        case 1: // 90 degrees (Landscape)
          x = raw_y;
          y = SCREEN_WIDTH - raw_x;
          break;
        case 2: // 180 degrees (Inverted Portrait)
          x = SCREEN_WIDTH - raw_x;
          y = SCREEN_HEIGHT - raw_y;
          break;
        case 3: // 270 degrees (Inverted Landscape)
          x = SCREEN_HEIGHT - raw_y;
          y = raw_x;
          break;
      }
      
      Serial.printf("Converted coordinates for drawing: X=%d, Y=%d\n", x, y);
      return true; // Success, valid coordinates obtained
    } else {
      Serial.println("Touch data received, but it's not a valid single touch point (first byte is not 0x01).");
      return false; // Not a valid touch point
    }
  } else {
    Serial.printf("I2C read error. Expected %d bytes, but got %d.\n", CHSC6X_READ_POINT_LEN, read_len);
    return false; // I2C read failed
  }
}

/**
 * @brief Standard Arduino setup function. Runs once at the beginning.
 */
void setup(){
  // Initialize serial communication for debugging output
  Serial.begin(115200);
  // Wait for serial port to connect. Useful for boards with native USB.
  while (!Serial) {
    delay(10); 
  }
  Serial.println("\n--- Setup Started ---");

  tft.init();
  Serial.println("TFT display initialized.");

  tft.setRotation(0); // Set initial screen rotation (0, 1, 2, or 3)
  Serial.printf("Screen rotation set to: %d\n", tft.getRotation());

  tft.fillScreen(TFT_WHITE);
  Serial.println("Screen filled with white color.");

  round_display_touch_init();
  
  Serial.println("--- Setup Finished. Entering loop. ---");
}

/**
 * @brief Standard Arduino loop function. Runs repeatedly.
 */
void loop() {
  // These variables will be updated by chsc6x_get_xy via reference
  int32_t touch_x = 0;
  int32_t touch_y = 0;

  // Check if the screen is being touched
  if (chsc6x_is_pressed()) {
    // If pressed, try to get the coordinates.
    // The function returns true if it successfully reads a valid coordinate.
    if (chsc6x_get_xy(touch_x, touch_y)) {
      Serial.printf("Drawing circle at final coordinates: X=%d, Y=%d\n\n", touch_x, touch_y);
      tft.fillCircle(touch_x, touch_y, 2, TFT_BLACK);
    }
  }
  
  // A small delay to prevent the loop from running too fast, 
  // which can be helpful for stability and reducing CPU load.
  delay(20); 
}
