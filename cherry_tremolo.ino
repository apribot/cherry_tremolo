#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=178,180
AudioAnalyzePeak         peak1;          //xy=311,106
AudioEffectEnvelope      envelope1;      //xy=337,178
AudioOutputI2S           i2s2;           //xy=495,180
AudioConnection          patchCord1(i2s1, 0, peak1, 0);
AudioConnection          patchCord2(i2s1, 0, envelope1, 0);
AudioConnection          patchCord3(envelope1, 0, i2s2, 0);
AudioConnection          patchCord4(envelope1, 0, i2s2, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=384,252
// GUItool: end automatically generated code


int transAvg = 0;
int inverseAvg = 0;

elapsedMillis fps;

int tickwindow = 0;
int tickpointer = 0;
int tickactive = 1;
int laststate = 1;

float peak = 0;
float fallRate = 0.001;
float linRead = 0.0;

const int myInput = AUDIO_INPUT_LINEIN;

void setup() {
  AudioMemory(12);

  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.volume(0.5);

  envelope1.attack(10);
  envelope1.decay(5000);
  envelope1.release(10);
  
  Serial.begin(9600);
}


void loop() {
  if (fps > 12) {
    if (peak1.available()) {
      fps = 0;

      // sound is exponential MAYBE???, so linear it
      linRead = sqrt(peak1.read());
      
      // grab the peak value and begin falling until the latest peak is higher than the current
      if(linRead > peak) {
        peak = linRead;
      }

      peak = peak - fallRate;

      // limit to only >=0 values
      if( peak < 0.0) {
        peak = 0;
      }

      // constrain to general input amplitide and invert for use in window size
      transAvg = constrain( int(peak * 1000.0), 0, 500);
      inverseAvg = 500 - transAvg;
      inverseAvg = map(inverseAvg, 0, 500, 5, 50);

      // calculate if we should be in an on or off state
      // and handle the counter used for the time window
      if (tickpointer > inverseAvg) {
        tickpointer = 0;
        tickactive = (tickactive + 1) % 2; // change state
      } else {        
        tickpointer += 1;
      }
      
      // if the on/off state has changed, trigger a on/off envelope
      if(laststate != tickactive) {
        Serial.print(inverseAvg);        
        Serial.print("\n");
        if(tickactive == 1) {
          envelope1.noteOn();
        } else {
          envelope1.noteOff();
        }
      }
      
      laststate = tickactive;
    }
  }
}
