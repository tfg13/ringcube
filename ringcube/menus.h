#include "Arduino.h"

/**
 * Contains most (but not all) of the gui-code
 * that displays the menus and reads the user input.
 */


void toggleAlarmMenu() {
  while (1) {
    // toggle alarm state
    alarm = !alarm;
    // show status
    clearDisplay();
    if (alarm) {
      manualControl(2, 0b00111111);//O
      manualControl(3, 0b00110111);//N
    } else {
      manualControl(1, 0b00111111);//O
      manualControl(2, 0b01110001);//F
      manualControl(3, 0b01110001);//F
    }
    // wait for further input (half second)
    long start = millis();
    while (1) {
      if (millis() - start > 750) {
	// alarm setting complete
	// if alarm is on, show time again before leaving
	if (alarm) {
	  displayFull(alarmHour * 100 + alarmMinute);
	  delay(750);
	}
	// back to main menu
	return;
      } else if (digitalRead(I_RIGHT) == LOW) {
	debounce(I_RIGHT);
	// toggle again (continue outer loop)
	break;
      }
    }
  }
}

void setAlarmMenu() {
  // set alarm time, display time + allow editing for each digit
  int8_t editing = 0;
  int8_t lastEditing = -1;
  int8_t lastHour = -1;
  int8_t lastMinute = -1;
  bool repaint = false;
  while (1) {
    if (editing != lastEditing || lastHour != alarmHour || lastMinute != alarmMinute || repaint) {
      clearDisplay();
      // display point if in single edit mode
      if (editing < 4) {
	setSpecialSymbol(false, 1 << editing, false);
      } else {
	setSpecialSymbol(false, 0, false);
      }
      displayFull(alarmHour * 100 + alarmMinute);
      lastEditing = editing;
      lastHour = alarmHour;
      lastMinute = alarmMinute;
      repaint = false;
    }
    // middle to toggle editing, l+r to switch
    if (digitalRead(I_LEFT) == LOW) {
      debounce(I_LEFT);
      editing--;
      if (editing < 0) {
	editing = 4;
      }
    } else if (digitalRead(I_RIGHT) == LOW) {
      debounce(I_RIGHT);
      editing++;
      if (editing > 4) {
	editing = 0;
      }
    } else if (digitalRead(I_MIDDLE) == LOW) {
      debounce(I_MIDDLE);
      // digit control or accept
      if (editing == 4) {
	return;
      } else {
	bool doneWithDigit = false;
	clearDisplay();
	while (1) {
	  setCursor(editing);
	  // display digit
	  if (editing == 0) {
	    displayRaw(alarmHour / 10);
	  } else if (editing == 1) {
	    displayRaw(alarmHour % 10);
	  } else if (editing == 2) {
	    displayRaw(alarmMinute / 10);
	  } else {
	    displayRaw(alarmMinute % 10);
	  }
	  // wait for input
	  while (1) {
	    if (digitalRead(I_MIDDLE) == LOW) {
	      debounce(I_MIDDLE);
	      // accept digit
	      doneWithDigit = true;
              repaint = true;
	      break;
	    } else if (digitalRead(I_LEFT) == LOW) {
	      debounce(I_LEFT);
	      // digit-- (beware leading 2 must be followed by 0-3)
	      if (editing == 0) {
		if (alarmHour < 10) {
		  alarmHour += 20;
		  if (alarmHour > 23) {
		    alarmHour = 23;
		  }
		} else {
		  alarmHour -= 10;
		}
	      } else if (editing == 1) {
		// last digit not zero -> substract
		if (alarmHour % 10 != 0) {
		  alarmHour--;
		} else {
		  alarmHour+= 9;
		  if (alarmHour / 10 == 2) {
		    alarmHour = 23;
		  }
		}
	      } else if (editing == 2) {
		if (alarmMinute < 10) {
		  alarmMinute += 50;
		} else {
		  alarmMinute -= 10;
		}
	      } else if (editing == 3) {
		// last digit not zero -> substract
		if (alarmMinute % 10 != 0) {
		  alarmMinute--;
		} else {
		  alarmMinute+= 9;
		}
	      }
	      break;
	    } else if (digitalRead(I_RIGHT) == LOW) {
	      debounce(I_RIGHT);
	      // digit++
	      if (editing == 0) {
		if (alarmHour < 20) {
		  alarmHour += 10;
		  if (alarmHour > 23) {
		    alarmHour = 23;
		  }
		} else {
		  alarmHour-= 20;
		}
	      } else if (editing == 1) {
		if (alarmHour % 10 != 9) {
		  // last digit not 9, add
		  alarmHour++;
		  if (alarmHour > 23) {
		    alarmHour = 20;
		  }
		} else {
		  alarmHour = (alarmHour / 10) * 10;
		}
	      } else if (editing == 2) {
		if (alarmMinute < 50) {
		  alarmMinute += 10;
		} else {
		  alarmMinute -= 50;
		}
	      } else if (editing == 3) {
		if (alarmMinute % 10 != 9) {
		  alarmMinute++;
		} else {
		  alarmMinute -= 9;
		}
	      }
	      break;
	    }
	  }
	  if (doneWithDigit) {
	    break;
	  }
	}
      }
    }
  }
}

// sets the alarm to ~8 hours from now
// rounds the minutes to 0, 15, 30 or 45.
// also enables the alarm
void quickSetAlarm() {
  alarmHour = rtc.getHour(dummy, dummy);
  alarmMinute = rtc.getMinute();
  // plus 8 hours
  alarmHour += 8;
  if (alarmHour > 23) {
    alarmHour -= 24;
  }
  // simplify minute
  if (alarmMinute > 52) {
    // round to next full hour
    alarmMinute = 0;
    alarmHour++;
    if (alarmHour == 24) {
      alarmHour = 0;
    }
  } else if (alarmMinute < 9) {
    // round to previous full hour
    alarmMinute = 0;
  } else if (alarmMinute >= 9 && alarmMinute <= 22) {
    alarmMinute = 15;
  } else if (alarmMinute > 22 && alarmMinute < 39) {
    alarmMinute = 30;
  } else if (alarmMinute >= 39) {
    alarmMinute = 45;
  }
  alarm = true;
  // show new status, ON + time
  clearDisplay();
  manualControl(2, 0b00111111);//O
  manualControl(3, 0b00110111);//N
  delay(750);
  displayFull(alarmHour * 100 + alarmMinute);
  delay(750);
  // back
}
