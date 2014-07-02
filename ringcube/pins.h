#include "Arduino.h"
// pin configuration

/*
 * main display (4*7segments)
 */
#define P_DISPLAY_POWER 4
#define P_DISPLAY_RX 0
#define P_DISPLAY_TX 1

/*
 * DCF (receiver for time signal)
 */
#define P_DCF_POWER 6
#define P_DCF_SIGNAL 5

/*
 * Audio (amplifier)
 */
#define P_AUDIO_SHUTDOWN 8
#define P_AUDIO_OUTPUT 9
// comms is I2C

/*
 * RTC (clock module)
 */
#define P_RTC_INTERRUPT 3
#define P_RTC_POWER A0
// comms is I2C

/*
 * SD card (audio files)
 */
#define P_SD_CS 10//chip select
#define P_SD_POWER 7
// comms is SPI

/*
 * Inputs
 */
#define I_LIGHT_INTERRUPT 2
#define I_LEFT A3
#define I_MIDDLE A2
#define I_RIGHT A1

/*
 * Setup inputs.
 * First thing to be executed after startup to limit power consumption and prevent damage to sensitive devices.
 * After this setup all pins are configured and all devices (except RTC) are powered down
 */
void setupPins() {
  // set everything
  pinMode(P_DISPLAY_POWER, OUTPUT);
  pinMode(P_DCF_POWER, OUTPUT);
  pinMode(P_DCF_SIGNAL, INPUT);
  pinMode(P_AUDIO_SHUTDOWN, OUTPUT);
  pinMode(P_AUDIO_OUTPUT, OUTPUT);
  pinMode(P_RTC_INTERRUPT, INPUT);
  pinMode(P_RTC_POWER, OUTPUT);
  pinMode(P_SD_CS, OUTPUT);
  pinMode(P_SD_POWER, OUTPUT);
  pinMode(I_LIGHT_INTERRUPT, INPUT);
  pinMode(I_LEFT, INPUT);
  pinMode(I_MIDDLE, INPUT);
  pinMode(I_RIGHT, INPUT);

  // disable everything except rtc
  digitalWrite(P_DISPLAY_POWER, LOW);
  digitalWrite(P_DCF_POWER, LOW);
  digitalWrite(P_AUDIO_SHUTDOWN, LOW);//low active TODO: Check!
  digitalWrite(P_AUDIO_OUTPUT, LOW);// use pwm? TODO: Check!
  digitalWrite(P_SD_CS, LOW);
  digitalWrite(P_SD_POWER, LOW);

  // explicitly enable rtc
  digitalWrite(P_RTC_POWER, HIGH);
  // enable I2C
  Wire.begin();
}
