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
// true while snoozing
bool snooze = false;
// holds the origin alarm time while snoozing
int8_t snoozeAlarmHour = 0;
int8_t snoozeAlarmMinute = 0;

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
 *
 * Ignore the return value for standard debouncing.
 * Returns true, if the button was successfully released within about 1 second.
 * If false is returned, the button may still be pressed. This is usefull for detecting
 * "long" clicks. It is recommended to call this method again after detecting a long click
 * to make sure the button is correctly debounced.
 */
bool debounce(int pin) {
  delay(10);
  // wait for button release
  long start = millis();
  bool result = false;
  while (millis() - start < 990) {
    if (digitalRead(pin) == HIGH) {
      result = true;
      break;
    }
  }
  // ignore falling edges for a moment
  delay(25);
  return result;
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
    // light button, display the time and wait for input.
    // sleep soon if nothing happens
    gotoActivePowerstate(POWERSTATE_DISPLAY);
    delay(100);
    // show time + colon
    setSpecialSymbol(true, 0, false);
    displayFull(rtc.getHour(dummy, dummy) * 100 + rtc.getMinute());

    if (snooze) {
      // still snoozing, so the alarm is technically still on.
      // 2 seconds to disable it by pressing any button
      long start = millis();
      while (1) {
	if (millis() - start > 2000) {
	  // back to sleep
	  break;
	} else if (digitalRead(I_LIGHT_INTERRUPT) == LOW) {
	  debounce(I_LIGHT_INTERRUPT);
	  alarm = false;
	} else if (digitalRead(I_RIGHT) == LOW) {
	  debounce(I_RIGHT);
	  alarm = false;
	} else if (digitalRead(I_MIDDLE) == LOW) {
	  debounce(I_MIDDLE);
	  alarm = false;
	} else if (digitalRead(I_LEFT) == LOW) {
	  debounce(I_LEFT);
	  alarm = false;
	}
	if (!alarm) {
	  // was turned off, disable snooze
	  snooze = false;
	  alarmHour = snoozeAlarmHour;
	  alarmMinute = snoozeAlarmMinute;
          clearDisplay();
          manualControl(1, 0b00111111);//O
	  manualControl(2, 0b01110001);//F
	  manualControl(3, 0b01110001);//F
	  delay(1000);
	  break;
	}
      }
    }
    // only enter main screen if snoozing is (now) off
    if (!snooze) {
      // wait for user input
      long displayStart = millis();
      while (1) {
	if (millis() - displayStart > 2000) {
	  break;
	}
	if (digitalRead(I_MIDDLE) == LOW) {
	  if (debounce(I_MIDDLE)) {
	    // normal click, go to set alarm time menu
	    setAlarmMenu();
	  } else {
	    // long click - quick set alarm
	    quickSetAlarm();
	  }
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
	  displayVoltage();
	  delay(2000);
	  //showStatus();
	  // back from showing status, reset display timer
	  setSpecialSymbol(true, 0, false);
	  displayFull(rtc.getHour(dummy, dummy) * 100 + rtc.getMinute());
	  displayStart = millis();
	}
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
      manageMusic();
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
	// first snooze?
	if (!snooze) {
	  snooze = true;
	  // preserve time
	  snoozeAlarmHour = alarmHour;
	  snoozeAlarmMinute = alarmMinute;
	}
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
	// was snoozing?
	if (snooze) {
	  snooze = false;
	  // reset original alarm time
	  alarmHour = snoozeAlarmHour;
	  alarmMinute = snoozeAlarmMinute;
	}
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
