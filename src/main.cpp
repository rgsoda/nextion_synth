
#include <Arduino.h>
#include "si5351.h"
#include "Wire.h"
#include "Nextion.h"

Si5351 si5351;


#define SI_SDA D2
#define SI_SCL D3
#define ENC_A  D5
#define ENC_B  D6

#define CD_A D0
#define CD_B D1
#define CD_C D4
#define CD_D D8

NexText freqText = NexText(0, 1, "FRQ");
NexButton bStep = NexButton(0, 5, "Bstep");
NexButton bUSB = NexButton(0, 6, "BUSB");
NexButton bLSB = NexButton(0, 7, "BLSB");
NexButton bCW = NexButton(0, 8, "BCW");
NexProgressBar pbPwr = NexProgressBar(0, 13, "pasek");

// Register a button object to the touch event list.  
NexTouch *nex_listen_list[] = {
  &bStep,
  &bUSB,
  &bLSB,
  &bCW,
  NULL
};




NexPicture m10m = NexPicture(0, 17, "M10M");
NexPicture m1m = NexPicture(0, 18, "M1M");
NexPicture m100k = NexPicture(0, 19, "M100k");
NexPicture m10k = NexPicture(0, 20, "M10k");
NexPicture m1k = NexPicture(0, 21, "M1k");
NexPicture m100Hz = NexPicture(0, 22, "M100Hz");
NexPicture m10Hz = NexPicture(0, 23, "M10Hz");
NexPicture m1Hz = NexPicture(0, 24, "M1Hz");

void clear_step_marker() {

  m10m.setPic(13);
  m1m.setPic(13);
  m100k.setPic(13);
  m10k.setPic(13);
  m1k.setPic(13);
  m100k.setPic(13);
  m10Hz.setPic(13);
  m1Hz.setPic(13);  
}


volatile unsigned long actFreq = 3500000;
unsigned long actStep = 1000;
unsigned long actPwr = 1;
boolean __change;

void set_actual_marker() {
  if(actStep == 1) m1Hz.setPic(12);
  if(actStep == 10) m10Hz.setPic(12);
  if(actStep == 100) m100Hz.setPic(12);
  if(actStep == 1000) m1k.setPic(12);
  if(actStep == 10000) m10k.setPic(12);
  if(actStep == 100000) m100k.setPic(12);
  if(actStep == 1000000) m1m.setPic(12);
  if(actStep == 10000000) m10m.setPic(12);  
}

void bStepPushCallback(void *ptr) {
  if (actStep == 10000000)  {
    actStep = 1;
  } else {
    actStep = actStep * 10;
  }
  clear_step_marker();
  set_actual_marker();

}



ICACHE_RAM_ATTR void handleInterrupt() {
  if (digitalRead(ENC_A) == digitalRead(ENC_B)){
	  actFreq = actFreq + actStep;
    actPwr = actPwr + 1;
  } else {
	  actFreq = actFreq - actStep;
    actPwr = actPwr - 1;
  }
  __change = 1;

}

void setup()
{
  Serial.begin(9600);

  nexInit();
  
  bStep.attachPush(bStepPushCallback, &bStep);

  clear_step_marker();
  m1k.setPic(12);

  pinMode(ENC_A, INPUT_PULLUP); 
  pinMode(ENC_B, INPUT_PULLUP);

  pinMode(CD_A, OUTPUT);
  pinMode(CD_B, OUTPUT);
  pinMode(CD_C, OUTPUT);
  pinMode(CD_D, OUTPUT);

  digitalWrite(CD_A, HIGH);
  digitalWrite(CD_B, HIGH); 
  digitalWrite(CD_C, HIGH); 
  digitalWrite(CD_D, HIGH);

  attachInterrupt(ENC_A, handleInterrupt, CHANGE);

  Wire.begin(SI_SDA, SI_SCL);


  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  si5351.set_freq(actFreq, SI5351_CLK2);
  __change = 1;

}

void format_commas(int n, char *out)
{
    int c;
    char buf[20];
    char *p;

    sprintf(buf, "%d", n);
    c = 2 - strlen(buf) % 3;
    for (p = buf; *p != 0; p++) {
       *out++ = *p;
       if (c == 1) {
           *out++ = '.';
       }
       c = (c + 1) % 3;
    }
    *--out = 0;
}

void update_state() {
  si5351.set_freq(actFreq, SI5351_CLK2);
  char buff[16];
  format_commas(actFreq, buff);
  freqText.setText(buff);
  pbPwr.setValue(actPwr);
  set_actual_marker();
}


void loop() {
  nexLoop(nex_listen_list);
  if(__change) {
    update_state();
    __change = 0;    
  }
}