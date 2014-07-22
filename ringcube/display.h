#include "Arduino.h"

// Helper functions for accessing the display

/**
 * Sends the given value directly to the display.
 */
void displayRaw(byte value) {
  Wire.beginTransmission(DISPLAY_ADDRESS1);
  Wire.write(value);
  Wire.endTransmission();
}

/**
 * Displays the given number with all four digits.
 * Adds zeros if necessary
 */
void displayFull(int fourDigits)
{
  Wire.beginTransmission(DISPLAY_ADDRESS1); // transmit to device #1
  Wire.write(fourDigits / 1000); //Send the left most digit
  fourDigits %= 1000; //Now remove the left most digit from the number we want to display
  Wire.write(fourDigits / 100);
  fourDigits %= 100;
  Wire.write(fourDigits / 10);
  fourDigits %= 10;
  Wire.write(fourDigits); //Send the right most digit
  Wire.endTransmission(); //Stop I2C transmission
}
/**
 * Turns on or off special symbols.
 * @param sep the ':' between HH:MM
 * @param dots the '.' after each digit, bits 0-3
 * @param upper the upper dots between digit 2 and 3
 */
void setSpecialSymbol(bool sep, int dots, bool upper) {
  uint8_t data = 0;
  if (sep) {
    data |= 0b00010000;
  }
  data |= dots & 0xF;
  if (upper) {
    data |= 0b00100000;
  }
  displayRaw(0x77);
  displayRaw(data);
}
/**
 * Sets the brightness of the display.
 * @param brightness 0 is dimmest, 255 brightest
 */
void setBrightness(uint8_t brightness) {
  displayRaw(0x7A);
  displayRaw(brightness);
}
/**
 * Clears the display.
 */
void clearDisplay() {
  displayRaw(0x76);
}
/**
 * Sets the cursor to the given digit.
 * @param pos digit to write next, 0-3
 */
void setCursor(uint8_t pos) {
  displayRaw(0x79);
  displayRaw(pos);
}
/**
 * Manually configures every segment of the given digit.
 * @param digit the digit to control, 0-3
 * @param bits bits for all 7 segments
 */
void manualControl(uint8_t digit, uint8_t bits) {
  if (digit >= 0 && digit <= 3) {
    displayRaw(0x7B + digit);
    displayRaw(bits);
  }
}
