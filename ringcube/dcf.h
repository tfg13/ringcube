#include "Arduino.h"
// decodes the dcf signal to get the current time and date

#define ONE_OR_ZERO_THRESHOLD_MS 150
#define MIN_HOLD_TIME 5
#define MAX_SEC_WAIT_MS 1100

int hold(int state) {
  long holdStart = millis();
  while (1) {
    if (millis() - holdStart > MIN_HOLD_TIME) {
      return 1;// hold was ok
    }
    if (digitalRead(P_DCF_SIGNAL) != state) {
      return 0;// unstable, bad!
    }
  }
}

/**
 * Reads the next bit received.
 * Returns 0 if a zero was received,
 * 1 if a 1 was received,
 * and 2 if nothing was received (minute mark)
 * 
 * Blocks for a maximum of MAX_SEC_WAIT_MS + 250
 */
int readBit() {
#ifdef DEBUG_DCF
     displayRaw(0x79);
     displayRaw(0x03);
#endif
  long start = millis();
  // wait for signal to be low
  while (1) {
    if (digitalRead(P_DCF_SIGNAL) == LOW && hold(LOW)) {
      break;
    }
    if (millis() - start > 250) {
      // error, return minute mark (fail fast in getTime-logic)   
#ifdef DEBUG_DCF
      displayRaw('4');
#endif
      return 2;
    }
  }
  start = millis();
  // wait for rising edge
  while (1) {
    if (digitalRead(P_DCF_SIGNAL) == HIGH && hold(HIGH)) {
      break;
    }
    if (millis() - start > MAX_SEC_WAIT_MS) {
      // found minute mark
#ifdef DEBUG_DCF
      displayRaw('2');
#endif
      return 2;
    }
  }
  start = millis();
  // measure time till falling edge
  while (1) {
    if (digitalRead(P_DCF_SIGNAL) == LOW && hold(LOW)) {
      break;
    }
    if (millis() - start > 250) {
      // error, return minute mark (fail fast in getTime-logic)
#ifdef DEBUG_DCF
     displayRaw('3');
#endif
      return 2;
    }
  }
  // decode duration to bit value
  long duration = millis() - start;
  if (duration < ONE_OR_ZERO_THRESHOLD_MS) {
#ifdef DEBUG_DCF
    displayRaw('0');
#endif
    return 0;
  } else {
#ifdef DEBUG_DCF
     displayRaw('1');
#endif
    return 1;
  }
}

/**
 * Validates the input, checks parity & constant fields
 */
int validateInput(uint8_t* input) {
  // check constant fields:
  if (input[0] != 1 || input[20] != 1) {
    return -1;
  }
  // check paritys
  // minutes
  int parity = input[21] + input[22] + input[23] + input[24] + input[25] + input[26] + input[27] % 2;
  if (input[28] != parity) {
    return -1;
  }
  // hours
  parity = input[29] + input[30] + input[31] + input[32] + input[33] + input[34] % 2;
  if (input[35] != parity) {
    return -1;
  }
  // date
  parity = input[36] + input[37] + input[38] + input[39] + input[40] + input[41];
  parity += input[42] + input[43] + input[44] + input[45] + input[46] + input[47];
  parity += input[48] + input[49] + input[50] + input[51] + input[52] + input[53];
  parity += input[54] + input[55] + input[56] + input[57];
  if (input[58] != parity) {
    return -1;
  }
  // everything was ok
  return 0;
}

void decodeTime(uint8_t* input) {
  int minute = input[21] + input[22] * 2 + input[23] * 4 + input[24] * 8 + input[25] * 10 + input[26] * 20 + input[27] * 40;
  int hour = input[29] + input[30] * 2 + input[31] * 4 + input[32] * 8 + input[33] * 10 + input[34] * 20;
  int day = input[36] + input[37] * 2 + input[38] * 4 + input[39] * 8 + input[40] * 10 + input[41] * 20;
  int month = input[45] + input[46] * 2 + input[47] * 4 + input[48] * 8 + input[49] * 10;
  int year = input[50] + input[51] * 2 + input[52] * 4 + input[53]*  8 + input[54] * 10 + input[55] * 20 + input[56] * 40 + input[57] * 80;
  int dow = input[42] + input[43] * 2 + input[44] * 4;
  // set
  rtc.setYear(year);
  rtc.setMonth(month);
  rtc.setDate(day);
  rtc.setDoW(dow);
  rtc.setHour(hour);
  rtc.setMinute(minute);
  rtc.setSecond(15);//this is not accurate, but will be corrected shortly
}

/**
 * Tries exactly once to read the time signal.
 * Blocks for a maximum time of 120 seconds.
 * Returns 0 upon success and -1 on failure.
 * On successfull reception, this method returns after the 1-second-mark is received, so the time is XX:YY:01
 * Remember that the DCF must be turned on 10 seconds before calling this as it needs time to callibrate!
 */
int getDCFTime() {
#ifdef DEBUG_DCF
    displayRaw('0x76');
#endif
  uint8_t received[60];
  // Wait for minute mark
  int triesLeft = 61;
  while (1) {
    if (readBit() == 2) {
      // found minute mark, continue
      break;
    }
    if (triesLeft-- < 0) {
      // couldn't find minute mark, abort
      return -1; 
    }
  }
  // receive 59 bits
  for (int i = 0; i < 59; i++) {
    int input = readBit();
#ifdef DEBUG_DCF
     displayRaw(0x79);
     displayRaw(0x00);
     displayRaw(i/10);
     displayRaw(i%10);
#endif
    if (input == 0 || input == 1) {
      // input ok
      received[i] = input;
    } else {
      // invalid input abort
      return -1;
    }
  }
  // receiving completed, decode and then wait for minute mark + sec 1 to return at the right time
  //if (validateInput((uint8_t*) &received)) {
    // error, constant fields or parity wrong
    //return -1;
  //}
  // decode time
  decodeTime((uint8_t*) &received);
  // wait for minute mark
  if (readBit() != 2) {
    // expected minute mark
    return -1;
  }
  // wait for rising edge of second-0 mark
  long sync = millis();
  while (1) {
    if (digitalRead(P_DCF_SIGNAL) == HIGH && hold(HIGH)) {
      // time reception complete
      rtc.setSecond(0); // this completes time reception and syncs the second
#ifdef DEBUG_DCF
     displayFull(3333);
     delay(2000);
     displayFull(2000 + rtc.getYear());
     delay(2000);
     bool centdummy;
     displayFull(rtc.getDate() * 100 + rtc.getMonth(centdummy));
     delay(2000);
     displayFull(rtc.getHour(centdummy, centdummy) * 100 + rtc.getMinute());
     delay(2000);
     displayFull(rtc.getSecond());
     delay(1000);
     displayFull(rtc.getSecond());
     delay(1000);
     displayFull(rtc.getSecond());
     delay(1000);
     displayFull(rtc.getSecond());
     delay(1000);
     displayFull(rtc.getSecond());
     delay(1000);
#endif
      return 0;
    }
    if (millis() - sync > MAX_SEC_WAIT_MS) {
      // rising edge missing
      return -1;
    }
  }
}
