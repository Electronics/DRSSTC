#include <MIDI.h>
#include "pitches.h"
#include "noteList.h"

#define ISR_SPEED 445700
#define MAX_POLYPHONY 10

uint32_t tone_counter[MAX_POLYPHONY]={0};
uint32_t tones[MAX_POLYPHONY]={0};
byte notes[MAX_POLYPHONY]={0};

#define GPIO2_H         (GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1<<2))
#define GPIO2_L         (GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1<<2))
#define GPIO2(x)        ((x)?GPIO2_H:GPIO2_L)

MIDI_CREATE_DEFAULT_INSTANCE();

void handleNoteOn(byte inChannel, byte inNote, byte inVelocity)
{
  for(int i=0;i<MAX_POLYPHONY;i++) {
    if(!tones[i]) {
      tones[i] = ISR_SPEED/sNotePitches[inNote];
      notes[i]=inNote;
      break;
    }
  }
}

void handleNoteOff(byte inChannel, byte inNote, byte inVelocity)
{
  for(int i=0;i<MAX_POLYPHONY;i++) {
    if(notes[i]==inNote) {
      notes[i]=0;
      tones[i]=0;
      tone_counter[i]=0;
      GPIO2_L;
      break;
    }
  }
}

void interrupter(void) {
  for(int i=0;i<MAX_POLYPHONY;i++) {
    if(tones[i]) tone_counter[i]++;
    if(tone_counter[i]==5) {
      GPIO2_L;
    }
    if(tone_counter[i]>tones[i]) {
      tone_counter[i]=0;
      GPIO2_H;
    }
  }
  timer0_write(ESP.getCycleCount()+100);
}

void setup() {
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
  pinMode(2, OUTPUT);
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.begin(1); // Launch MIDI and listen to channel 1
  
  noInterrupts();
  timer0_isr_init();
  timer0_attachInterrupt(interrupter);
  timer0_write(ESP.getCycleCount()+500);
  interrupts();
}

void loop() {
  MIDI.read();
}
