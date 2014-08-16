// music playback stuff
#include <Wire.h>

SdFat sd;

Adafruit_TPA2016 audioamp = Adafruit_TPA2016();
TMRpcm tmrpcm;   // create an object for use in this sketch

// if there is a problem with the sd card, a simple tone will be generated
boolean sdError = false;
long nextBeep = 0;

// short double beep
void beep() {
  tone(9, 1500);
  delay(25);
  noTone(9);
  delay(100);
  tone(9, 1500);
  delay(25);
  noTone(9);
  nextBeep = millis() + 1000;
}

void ring() {
  tmrpcm.speakerPin = 9;
  audioamp.enableChannel(true, false);
  audioamp.setLimitLevel(5);
  audioamp.setGain(4);
  
  SPI.begin();
  // see if the card is present and can be initialized
  sdError = !sd.begin(P_SD_CS, SPI_HALF_SPEED);

  if (sdError) {
    // play beep tone
    audioamp.setLimitLevel(1);
    audioamp.setGain(1);
    beep();
  } else {
    tmrpcm.setVolume(3);
    tmrpcm.play("music");
  }
}

void manageMusic() {
  if (sdError) {
    if (millis() > nextBeep) {
      beep();
    }
  }
}

void silence() {
  if (sdError) {
    noTone(9);
    nextBeep = 0;
    sdError = false;
  } else {
    tmrpcm.disable();
  }
  SPI.end();
}
