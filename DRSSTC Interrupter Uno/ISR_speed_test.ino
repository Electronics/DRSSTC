#include <MIDI.h>
#include "pitches.h"
#include "noteList.h"

#define ISR_SPEED 80500
#define MAX_POLYPHONY 5

uint16_t tone_counter[MAX_POLYPHONY]={0};
uint16_t tones[MAX_POLYPHONY]={0};
byte notes[MAX_POLYPHONY]={0};

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
      PORTB &= ~(0b00100000);
      break;
    }
  }
}

void setup() {
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.begin(1); // Launch MIDI and listen to channel 1
  
  noInterrupts();
  TIMSK0 &= ~_BV(TOIE0); // disable timer0 overflow interrupt
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;
  OCR2A = 10; // Compare Match register
  TCCR2A |= (1 << WGM21); // CTC mode
  TCCR2B |= (1 << CS21 | 1 << CS20); // 32 prescaler
  TIMSK2 |= (1 << OCIE2A); // enable timer compare interrupt
  interrupts();
}

void loop() {
  MIDI.read();
}

ISR(TIMER2_COMPA_vect) {
  for(int i=0;i<MAX_POLYPHONY;i++) {
    if(tones[i]) tone_counter[i]++;
    if(tone_counter[i]==5) {
      PORTB &= ~(0b00100000);
    }
    if(tone_counter[i]>tones[i]) {
      tone_counter[i]=0;
      PORTB |= (0b00100000);
    }
  }
}
