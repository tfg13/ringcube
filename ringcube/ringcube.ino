#include <Wire.h>
#include <Time.h>

#define DEBUG_DCF

#include "pins.h"
#include "constants.h"

void displayRaw(byte value) {
  Wire.beginTransmission(DISPLAY_ADDRESS1);
  Wire.write(value);
  Wire.endTransmission();
}
void displayFull(int tempCycles)
{
  Wire.beginTransmission(DISPLAY_ADDRESS1); // transmit to device #1
  Wire.write(tempCycles / 1000); //Send the left most digit
  tempCycles %= 1000; //Now remove the left most digit from the number we want to display
  Wire.write(tempCycles / 100);
  tempCycles %= 100;
  Wire.write(tempCycles / 10);
  tempCycles %= 10;
  Wire.write(tempCycles); //Send the right most digit
  Wire.endTransmission(); //Stop I2C transmission
}

#include <Adafruit_TPA2016.h>
#include <TMRpcm.h>
#include <SPI.h>
#include <SD.h>
#include <DS3231.h>


DS3231 rtc = DS3231();


#include "dcf.h"
#include "timers.h"
#include "audio.h"
#include "power.h"


void setup() {
  setupPins();
}

void loop() {
  gotoActivePowerstate(POWERSTATE_DISPLAY);
  delay(100);
  bool dummy;
  displayRaw(0x77);  // Decimal control command
  displayRaw(0b00010000);
  displayRaw(0x7A);  // brightness
  displayRaw(255);
  displayFull(rtc.getHour(dummy, dummy) * 100 + rtc.getMinute());
  delay(3000);
  rtc.turnOffAlarm(1);
  rtc.turnOffAlarm(2);
  deepsleep();
}
