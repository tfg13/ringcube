// music playback stuff
#include <Wire.h>

SdFat sd;

Adafruit_TPA2016 audioamp = Adafruit_TPA2016();
TMRpcm tmrpcm;   // create an object for use in this sketch

void ring() {
  tmrpcm.speakerPin = 9;
  audioamp.enableChannel(true, false);
  audioamp.setLimitLevel(5);
  audioamp.setGain(4);
  
  SPI.begin();
  if (!sd.begin(P_SD_CS, SPI_HALF_SPEED)) {  // see if the card is present and can be initialized:
    // todo: better handling
    displayFull(6666);
    delay(500);
    return;   // don't do anything more if not
  }
  tmrpcm.setVolume(3);
  tmrpcm.play("music");
}

void silence() {
  tmrpcm.disable();
  SPI.end();
}
