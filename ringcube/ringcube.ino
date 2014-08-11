#include <Wire.h>
#include <Time.h>

#define DEBUG_DCF

#include "pins.h"
#include "constants.h"

#include <Adafruit_TPA2016.h>
#include <TMRpcm.h>
#include <SPI.h>
#include <SdFat.h>
#include <DS3231.h>

// object to access the real-time clock module, a Chronodot.
DS3231 rtc = DS3231();

#include "display.h"
#include "dcf.h"
#include "audio.h"


// what triggered the last wakeup
uint8_t waketrigger;
// certain rtc reads produce additional info we don't need - store them here
bool dummy;
// if alarm is turned on
bool alarm = false;
// full hour of the alarm (24-hour format)
int8_t alarmHour = 0;
// minute of the alarm
int8_t alarmMinute = 0;

#include "menus.h"
#include "timers.h"
#include "power.h"
#include "diag.h"

/**
 * Used for debouncing all buttons.
 * This is called immediately after a button press is registered.
 * It closely monitors the button and waits a bit to prevent oscillating
 * or continued readings from being interpreted as yet another input.
 *
 * Works for all buttons that are low-active (which are all).
 * Supply the pressed button as the first argument.
 */
void debounce(int pin) {
  delay(10);
  // wait for button release
  long start = millis();
  while (millis() - start < 1990) {
    if (digitalRead(pin) == HIGH) {
      break;
    }
  }
  // ignore falling edges for a moment
  delay(25);
}

/**
 * Usual arduino setup funciton.
 * Code starts running here.
 * Does some initial pin-assignments.
 * It is important to put the device to sleep shortly after, because the
 * inital pin assigments are suboptimal and can waste a lot of power.
 */
void setup() {
  setupPins();
  // first start is like pressing the light button
  waketrigger = WAKEREASON_LIGHT;
  // diag/reset?
  if (!digitalRead(I_MIDDLE) && !digitalRead(I_LEFT) && !digitalRead(I_RIGHT)) {
    resetRTC();
  }
  if (!digitalRead(I_LEFT) && !digitalRead(I_RIGHT)) {
    diag();
  } else if (!digitalRead(I_MIDDLE) && !digitalRead(I_RIGHT)) {
    diagOSF();
  }
}

/**
 * Usual arduino main loop.
 * This is repeatedly called after setup()
 */
void loop() {
  // device just woke up from deepsleep.
  // find out the reason for this
  if (waketrigger == WAKEREASON_LIGHT) {
    // light button. display the time and wait for input.
    // sleep soon if nothing happens
    gotoActivePowerstate(POWERSTATE_DISPLAY);
    delay(100);
    // show time + colon
    setSpecialSymbol(true, 0, false);
    displayFull(rtc.getHour(dummy, dummy) * 100 + rtc.getMinute());
    // wait for user input
    long displayStart = millis();
    while (1) {
      if (millis() - displayStart > 2000) {
	break;
      }
      if (digitalRead(I_MIDDLE) == LOW) {
	debounce(I_MIDDLE);
	setAlarmMenu();
	// back from menu, reset display timer
	setSpecialSymbol(true, 0, false);
	displayFull(rtc.getHour(dummy, dummy) * 100 + rtc.getMinute());
	displayStart = millis();
      } else if (digitalRead(I_RIGHT) == LOW) {
	debounce(I_RIGHT);
	toggleAlarmMenu();
	// back from menu, reset display timer
	setSpecialSymbol(true, 0, false);
	displayFull(rtc.getHour(dummy, dummy) * 100 + rtc.getMinute());
	displayStart = millis();
      } else if (digitalRead(I_LEFT) == LOW) {
	debounce(I_LEFT);
	//showStatus();
	// back from showing status, reset display timer
	setSpecialSymbol(true, 0, false);
	displayFull(rtc.getHour(dummy, dummy) * 100 + rtc.getMinute());
	displayStart = millis();
      }
    }
  } else if (waketrigger == WAKEREASON_TIMER) {
    // alarm
    // give components some time to wake up
    gotoActivePowerstate(POWERSTATE_RINGING);
    delay(100);
    // show time
    setSpecialSymbol(true, 0, false);
    bool dummy;
    displayFull(rtc.getHour(dummy, dummy) * 100 + rtc.getMinute());
    // make some noise
    ring();
    // wait for light button
    while (digitalRead(I_LIGHT_INTERRUPT) == HIGH) {
    }
    debounce(I_LIGHT_INTERRUPT);
    // silence, save power
    silence();
    gotoActivePowerstate(POWERSTATE_DISPLAY);
    delay(100);
    setSpecialSymbol(true, 0, false);
    displayFull(rtc.getHour(dummy, dummy) * 100 + rtc.getMinute());
    // if nothing happens withing 3 seconds snooze is enabled.
    // otherwise alarm is turned off
    long alarmOffTimer = millis();
    while (1) {
      if (millis() - alarmOffTimer > 3000) {
	// snooze!
	// move alarm 3 minutes to the future
	if (alarmMinute < 57) {
	  alarmMinute += 3;
	} else {
	  alarmMinute -= 57;
	  if (alarmHour == 23) {
	    alarmHour = 0;
	  } else {
	    alarmHour += 1;
	  }
	}
	// now go back to sleep
	break;
      }
      if (digitalRead(I_LIGHT_INTERRUPT) == LOW) {
	// disable alarm
	alarm = false;
	clearDisplay();
	manualControl(1, 0b00111111);//O
	manualControl(2, 0b01110001);//F
	manualControl(3, 0b01110001);//F
	delay(3000);
	// good morning, turn cube off
	break;
      }
    }
  }
  // done with everything, sleep again
  waketrigger = deepsleep();
}
