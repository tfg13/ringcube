#include "Arduino.h"
#include <avr/sleep.h>

// manages power-states and switching between them
// also guarantees that the system is never send to sleep without a wakeup-timer being set


// current powerstate, after initialization this is POWERSTATE_ON for background operations
// note: this variable is never set to POWERSTATE_SLEEP (makes no sense, since it cannot be read while sleeping)
int powerstate = POWERSTATE_ON;

// reason for last wakeup, volatile because used in an ISR
volatile uint8_t wakeReason = 0;

/**
 * ISR for light button
 */
void wakeLight() {
  // cancel sleep as a precaution
  sleep_disable();
  // must do this as the pin will probably stay low for a while
  detachInterrupt (0);
  detachInterrupt (1);
  wakeReason = WAKEREASON_LIGHT;
}

/**
 * ISR for timer interrupts
 */
void wakeTimer() {
  // cancel sleep as a precaution
  sleep_disable();
  // must do this as the pin will probably stay low for a while
  detachInterrupt (0);
  detachInterrupt (1);
  wakeReason = WAKEREASON_TIMER;
}

/**
 * Handles the low-level sleep stuff.
 * Disables I2C, ADC, enables interrupts and switches to deepest sleepmode.
 * Does not check for set timers.
 * Blocks until sleep is over
 */
void deepsleep_internal() {
  // disable I2C
  TWCR &= ~(_BV(TWEN) | _BV(TWIE) | _BV(TWEA));
  PRR |= (1<<PRSPI) | (1<<PRTWI);
  // disable I2C pullups
  digitalWrite (A4, LOW);
  digitalWrite (A5, LOW);
  // disable SPI
  
  digitalWrite(P_RTC_POWER, LOW);
  pinMode(P_RTC_POWER, INPUT);
  //digitalWrite(10, HIGH);
  
  // disable ADC subsystem
  ADCSRA = 0; 
  
  // deepest sleepmode
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
  sleep_enable();
  
  // Do not interrupt before we go to sleep, or the
  // ISR will detach interrupts and we won't wake.
  noInterrupts ();

  // will be called when pin D2 goes low  
  attachInterrupt(0, wakeLight, LOW);
  attachInterrupt(1, wakeTimer, LOW);

  // turn off brown-out detection (low-bat alarm should prevent brown-outs)
  MCUCR = bit (BODS) | bit (BODSE);  // turn on brown-out enable select
  MCUCR = bit (BODS);        // this must be done within 4 clock cycles of above
  interrupts();
  sleep_cpu ();    // sleep within 3 clock cycles of above
  
  // re-enable I2C & SPI
  PRR &= (1<<PRTIM0) | (1<<PRTIM1) | (1<<PRTIM2) | (1<<PRADC) | (1<<PRUSART0);

  disableAlarm();

  // begin re-enabling stuff
  // power rtc
  digitalWrite (P_RTC_POWER, HIGH);
  
  Wire.begin();
}

/**
 * Switches to another (active) powerstate.
 * Cannot switch to sleep!
 */
void gotoActivePowerstate(uint8_t nextPowerstate) {
  if (nextPowerstate == POWERSTATE_SLEEP) {
    // use deepsleep for this
    return;
  }
  switch (nextPowerstate) {
    case POWERSTATE_ON:
      // disable everything except rtc
      digitalWrite(P_DISPLAY_POWER, LOW);
      pinMode(P_DISPLAY_POWER, INPUT);
      digitalWrite(P_DCF_POWER, LOW);
      digitalWrite(P_AUDIO_SHUTDOWN, LOW);
      digitalWrite(P_AUDIO_OUTPUT, LOW);
      pinMode(P_AUDIO_OUTPUT, INPUT);
      digitalWrite(P_SD_CS, LOW);
      digitalWrite(P_SD_POWER, LOW);
      digitalWrite(13, LOW);
      digitalWrite(12, LOW);
      digitalWrite(11, LOW);
      pinMode(13, INPUT);
      pinMode(12, INPUT);
      pinMode(11, INPUT);
      break;
    case POWERSTATE_DISPLAY:
      pinMode(P_DISPLAY_POWER, OUTPUT);
      digitalWrite(P_DISPLAY_POWER, HIGH);
      digitalWrite(P_DCF_POWER, LOW);
      digitalWrite(P_AUDIO_SHUTDOWN, LOW);
      digitalWrite(P_AUDIO_OUTPUT, LOW);
      pinMode(P_AUDIO_OUTPUT, INPUT);
      digitalWrite(P_SD_CS, LOW);
      digitalWrite(P_SD_POWER, LOW);
      digitalWrite(13, LOW);
      digitalWrite(12, LOW);
      digitalWrite(11, LOW);
      pinMode(13, INPUT);
      pinMode(12, INPUT);
      pinMode(11, INPUT);
      break;
    case POWERSTATE_RINGING:
      pinMode(P_DISPLAY_POWER, OUTPUT);
      digitalWrite(P_DISPLAY_POWER, HIGH);
      digitalWrite(P_DCF_POWER, LOW);
      digitalWrite(P_AUDIO_SHUTDOWN, HIGH);
      pinMode(P_AUDIO_OUTPUT, OUTPUT);
      pinMode(13, OUTPUT);
      pinMode(12, OUTPUT);
      pinMode(11, OUTPUT);
      digitalWrite(P_SD_POWER, HIGH);
      break;
    case POWERSTATE_DCF:
#ifdef DEBUG_DCF
      pinMode(P_DISPLAY_POWER, OUTPUT);
      digitalWrite(P_DISPLAY_POWER, HIGH);
#else
      digitalWrite(P_DISPLAY_POWER, LOW);
      pinMode(P_DISPLAY_POWER, INPUT);
#endif
      digitalWrite(P_DCF_POWER, HIGH);
      digitalWrite(P_AUDIO_SHUTDOWN, LOW);
      digitalWrite(P_AUDIO_SHUTDOWN, LOW);
      digitalWrite(P_AUDIO_OUTPUT, LOW);
      pinMode(P_AUDIO_OUTPUT, INPUT);
      digitalWrite(P_SD_CS, LOW);
      digitalWrite(P_SD_POWER, LOW);
      digitalWrite(13, LOW);
      digitalWrite(12, LOW);
      digitalWrite(11, LOW);
      pinMode(13, INPUT);
      pinMode(12, INPUT);
      pinMode(11, INPUT);
      break;
  }
  powerstate = nextPowerstate;
}

/**
 * Puts the device into deepsleep, preserving power.
 * Calls timer functions to guarantee a wakeup withing the next hour.
 * Blocks until sleeping is over. (max 1h)
 * Returns the reason for wakeup.
 * This is either WAKEREASON_LIGHT or WAKEREASON_TIMER
 */
uint8_t deepsleep() {
  // goto lowest powerstate first
  gotoActivePowerstate(POWERSTATE_ON);

  // ensure wakeup within the next hour
  resetWakeupTimer();

  // do real sleeping
  deepsleep_internal();
  return wakeReason;
}
