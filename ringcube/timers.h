#include "Arduino.h"

/**
 * Re-configures the timers to automatically wake the system up, if required.
 * This function must be called before going to sleep. Otherwise, the device
 * cannot wake up without user interaction (that is, pressing the light button)
 *
 * Currenty this function only configures and enabled the alarm timer, if the alarm is enabled.
 * There is a second timer that can be used for periodic timekeeping tasks like correcty the rtc with the dcf.
 * While this is not implemented at this point it may be in the future.
 */
void resetWakeupTimer() {
  // only 1 timer for now
  if (alarm) {
    rtc.setA2Time(0, alarmHour, alarmMinute, 0b01000000, false, false, false);
    rtc.turnOnAlarm(2);
  } else {
    rtc.turnOffAlarm(2);
  }
  // retrieve alarm flag, otherwise the previous alarm may still be active
  rtc.checkIfAlarm(2);
}

/**
 * Disables all alarms.
 * Must be called after device woke up from an alarm.
 * This clears the alarm flag of the rtc and therefore prevents the
 * device from immediately waking up again.
 */
void disableAlarm() {
  rtc.turnOffAlarm(2);
  rtc.checkIfAlarm(2);
}
