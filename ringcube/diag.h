// displays diagnostics

/*
 * Information is displayed in the following order:
 * 
 * - oscillator status (must be ON)
 * - current time HH:MM
 * - control register values
 * - status register values
 * - A1 day
 * - A1 HH:MM
 * - A1 second
 * - A1 bits
 * - A1 flags
 * - A1 enabled?
 * - A2 day
 * - A2 HH:MM
 * - A2 bits
 * - A2 flags
 * - A2 enabled?
 * - temperature
 * - seconds (repeated forever)
 */

void diag() {
  // display OSF (oscillator stop flag)
  if (rtc.oscillatorCheck()) {
    manualControl(2, 0b00111111);//O
    manualControl(3, 0b00110111);//N
  } else {
    manualControl(1, 0b00111111);//O
    manualControl(2, 0b01110001);//F
    manualControl(3, 0b01110001);//F
  }
  // display current time according to rtc
  delay(2000);
  displayFull(rtc.getHour(dummy, dummy) * 100 + rtc.getMinute());
  delay(2000);
  // display status, control register
  byte reg = 0;
  displayFull(rtc.readControlByte(0));
  delay(2000);
  displayFull(rtc.readControlByte(1));
  // display alarm 1 time
  delay(2000);
  byte day, hour, minute, second, bits;
  bool dy, h12, pm;
  rtc.getA1Time(day, hour, minute, second, bits, dy, h12, pm);
  displayFull(day);
  delay(2000);
  displayFull(hour * 100 + minute);
  delay(2000);
  displayFull(second);
  delay(2000);
  displayFull(bits);
  delay(2000);
  byte flags = 0;
  flags |= dy;
  flags |= h12 << 1;
  flags |= pm << 2;
  displayFull(flags);
  // display alarm 1 status
  delay(2000);
  if (rtc.checkAlarmEnabled(1)) {
    manualControl(2, 0b00111111);//O
    manualControl(3, 0b00110111);//N
  } else {
    manualControl(1, 0b00111111);//O
    manualControl(2, 0b01110001);//F
    manualControl(3, 0b01110001);//F
  }
  delay(2000);
  if (rtc.checkIfAlarm(1)) {
    manualControl(2, 0b00111111);//O
    manualControl(3, 0b00110111);//N
  } else {
    manualControl(1, 0b00111111);//O
    manualControl(2, 0b01110001);//F
    manualControl(3, 0b01110001);//F
  }
  // display alarm 2 time
  delay(2000);
  rtc.getA2Time(day, hour, minute, bits, dy, h12, pm);
  displayFull(day);
  delay(2000);
  displayFull(hour * 100 + minute);
  delay(2000);
  displayFull(bits);
  delay(2000);
  flags = 0;
  flags |= dy;
  flags |= h12 << 1;
  flags |= pm << 2;
  displayFull(flags);
  // display alarm 2 status
  delay(2000);
  if (rtc.checkAlarmEnabled(2)) {
    manualControl(2, 0b00111111);//O
    manualControl(3, 0b00110111);//N
  } else {
    manualControl(1, 0b00111111);//O
    manualControl(2, 0b01110001);//F
    manualControl(3, 0b01110001);//F
  }
  delay(2000);
  if (rtc.checkIfAlarm(2)) {
    manualControl(2, 0b00111111);//O
    manualControl(3, 0b00110111);//N
  } else {
    manualControl(1, 0b00111111);//O
    manualControl(2, 0b01110001);//F
    manualControl(3, 0b01110001);//F
  }
  // display temp
  delay(2000);
  byte temp = (byte) rtc.getTemperature();
  displayFull(temp);
  delay(2000);
  
  // display seconds for ever
  while (1) {
    displayFull(rtc.getSecond());
    delay(1000);
  }
}

// resets the rtc & triggers dcf update
void resetRTC() {
  gotoActivePowerstate(POWERSTATE_DISPLAY);
  delay(100);
  displayRaw('-');
  displayRaw('-');
  displayRaw('-');
  displayRaw('-');
  delay(5000);
  gotoActivePowerstate(POWERSTATE_DCF);
#ifdef DEBUG_DCF
  for (int8_t i = 60; i >= 0; i--) {
    displayFull(i);
    delay(1000);
  }
#else
  delay(60000);
#endif
  rtc.reconfigureForAlarms();
  rtc.setA1Time(1, 0, 0, 0, 0, false, false, false);
  rtc.setA2Time(1, 0, 0, 0, false, false, false);
  rtc.turnOffAlarm(1);
  rtc.turnOffAlarm(2);
  rtc.checkIfAlarm(1);
  rtc.checkIfAlarm(1);
  rtc.checkIfAlarm(2);
  rtc.checkIfAlarm(2);
  getDCFTime();
}

void diagOSF() {
  gotoActivePowerstate(POWERSTATE_DISPLAY);
  delay(100);
  // display OSF (oscillator stop flag)
  if (rtc.oscillatorCheck()) {
    manualControl(2, 0b00111111);//O
    manualControl(3, 0b00110111);//N
  } else {
    manualControl(1, 0b00111111);//O
    manualControl(2, 0b01110001);//F
    manualControl(3, 0b01110001);//F
  }
  // switch oscillator on
  rtc.setSecond(10);
  delay(1000);
  if (rtc.oscillatorCheck()) {
    manualControl(2, 0b00111111);//O
    manualControl(3, 0b00110111);//N
  } else {
    manualControl(1, 0b00111111);//O
    manualControl(2, 0b01110001);//F
    manualControl(3, 0b01110001);//F
  }
}

void displayVoltage() {
  // dot after first digit
  setSpecialSymbol(false, 1, false);
  displayFull(readVcc());
}
