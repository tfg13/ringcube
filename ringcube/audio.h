// music playback stuff
#include <Wire.h>

Adafruit_TPA2016 audioamp = Adafruit_TPA2016();
TMRpcm tmrpcm;   // create an object for use in this sketch

void ring() {
  tmrpcm.speakerPin = 9;
  audioamp.enableChannel(true, false);
  audioamp.setLimitLevel(5);
  audioamp.setGain(0);
  
  if (!SD.begin(P_SD_CS)) {  // see if the card is present and can be initialized:
    // todo: better handling
    displayFull('6666');
    return;   // don't do anything more if not
  }
  tmrpcm.setVolume(3);
  tmrpcm.play("music"); //the sound file "music" will play each time the arduino powers up, or is reset
}

void silence() {
  tmrpcm.disable();
}
