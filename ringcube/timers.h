#include "Arduino.h"

/**
 * Re-configures the normal timer to wake up the system within the next hour.
 * During normal operation, this function sets the wakeup time to exactly one hour in the future.
 * If the computed wakeup time interferes with the current alarm time or configured dcf time it will be shortened.
 */
void resetWakeupTimer() {
  // only 1 timer for now
  if (alarm) {
    rtc.setA2Time(0, alarmHour, alarmMinute, 0b01000000, false, false, false);
    rtc.turnOnAlarm(2);
    rtc.checkIfAlarm(2);
  } else {
    rtc.turnOffAlarm(2);
  }
}
