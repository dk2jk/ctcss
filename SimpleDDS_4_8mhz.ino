/*
   Simple DDS Signal Generator

   2017/6/20 by morecat_lab

   based on http://interface.khm.de/index.php/lab/interfaces-advanced/arduino-dds-sinewave-generator/
   KHM 2009 /  Martin Nawrath
   Kunsthochschule fuer Medien Koeln
   Academy of Media Arts Cologne

   modifiziert: dk2jk 10.05.2018 ( nur ein Kanal )
*/
#include "avr/pgmspace.h"
#include "Arduino.h"
#define SINOUT1   11 // OC2A (PB3) require filter
#define SQUOUT11  7 //    - (PD7)
#define LED       13
#define ENABLE    2

// table of 256 sine values / one sine period / stored in flash memory
const unsigned char sine256[] PROGMEM  = {
  127, 130, 133, 136, 139, 143, 146, 149, 152, 155, 158, 161, 164, 167, 170, 173, 176, 178, 181, 184, 187, 190, 192, 195, 198, 200, 203, 205, 208, 210, 212, 215, 217, 219, 221, 223, 225, 227, 229, 231, 233, 234, 236, 238, 239, 240,
  242, 243, 244, 245, 247, 248, 249, 249, 250, 251, 252, 252, 253, 253, 253, 254, 254, 254, 254, 254, 254, 254, 253, 253, 253, 252, 252, 251, 250, 249, 249, 248, 247, 245, 244, 243, 242, 240, 239, 238, 236, 234, 233, 231, 229, 227, 225, 223,
  221, 219, 217, 215, 212, 210, 208, 205, 203, 200, 198, 195, 192, 190, 187, 184, 181, 178, 176, 173, 170, 167, 164, 161, 158, 155, 152, 149, 146, 143, 139, 136, 133, 130, 127, 124, 121, 118, 115, 111, 108, 105, 102, 99, 96, 93, 90, 87, 84, 81, 78,
  76, 73, 70, 67, 64, 62, 59, 56, 54, 51, 49, 46, 44, 42, 39, 37, 35, 33, 31, 29, 27, 25, 23, 21, 20, 18, 16, 15, 14, 12, 11, 10, 9, 7, 6, 5, 5, 4, 3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 5, 5, 6, 7, 9, 10, 11, 12, 14, 15, 16, 18, 20, 21, 23, 25, 27, 29, 31,
  33, 35, 37, 39, 42, 44, 46, 49, 51, 54, 56, 59, 62, 64, 67, 70, 73, 76, 78, 81, 84, 87, 90, 93, 96, 99, 102, 105, 108, 111, 115, 118, 121, 124

};
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

char buf[32];

#define REFCLK (31376.6/2.0)       // 8 mhz arduino mini pro
volatile unsigned long phaccu_a;   // phase accumulator
volatile unsigned long tword_a;    // dds tuning word m for OC2A
volatile byte icnt_a;              // var inside interrupt
volatile unsigned long phaccu_b;   // phase accumulator
volatile unsigned long tword_b;    // dds tuning word m for OC2B
byte freqIndex;

float frequenzTabelle[16] =
  ///*6543*/  Brücken an Port 6543 gegen GND
{ /*0000*/ 67.0  ,/*0001*/ 71.9  ,/*0010*/ 74.4  ,/*0011*/ 85.4  ,
  /*0100*/ 88.5  ,/*0101*/ 91.5  ,/*0110*/ 94.8  ,/*0111*/ 100.0 ,
  /*1000*/ 103.5 ,/*1001*/ 110.9 ,/*1010*/ 114.8 ,/*1011*/ 123.0 ,
  /*1100*/ 127.3 ,/*1101*/ 131.8 ,/*1110*/ 136.5 ,/*1111*/ 141.3
};

unsigned long tick( int index)
{ return pow(2, 32) * frequenzTabelle[index] / REFCLK;
}

byte readKanalNr()
{
  pinMode( 3, INPUT_PULLUP);
  pinMode( 4, INPUT_PULLUP);
  pinMode( 5, INPUT_PULLUP);
  pinMode( 6, INPUT_PULLUP);
  byte y = 0;
  if (digitalRead(6) == 0 ) y = 0x8;
  if (digitalRead(5) == 0 ) y = y + 0x4;
  if (digitalRead(4) == 0 ) y = y + 0x2;
  if (digitalRead(3) == 0 ) y = y + 0x1;
  return y;
}

void setup()
{
  // sets the digital pin as output
  pinMode(LED, OUTPUT);
  pinMode(SINOUT1, OUTPUT);
  pinMode(SQUOUT11, OUTPUT);
  pinMode(ENABLE, INPUT_PULLUP);
  cbi (TIMSK0, TOIE0);             // disable Timer0 !!! delay() is now not available
  Setup_timer2();
  freqIndex = readKanalNr();
  setup_SineFreq();
}


void loop() {
  static byte index_alt = 0;
  bool en=digitalRead(ENABLE) == HIGH;
  digitalWrite(LED,en==0);
  en ? cbi (TIMSK2, TOIE2) : sbi (TIMSK2, TOIE2);
  // enable/disable Timer2 Interrupt
  byte index_neu = readKanalNr();
  if ( index_alt == index_neu )
  { //keine ae
  }
  else
  { freqIndex = index_neu;
    setup_SineFreq();
    index_alt = index_neu;
  }
}

void setup_SineFreq() {
  cbi (TIMSK2, TOIE2);              // disable Time2 Interrupt
  tword_a = tick(freqIndex) ; //sin_freqtab[freqIndex].tick;
  phaccu_a = 0;
  sbi (TIMSK2, TOIE2);              // enable Timer2 Interrupt
}

void shift_SinPhase( byte angle) {
  cbi (TIMSK2, TOIE2);              // disable Time2 Interrupt
  phaccu_a = phaccu_a + (angle * 0x1000000); // shift 1/256 degree
  sbi (TIMSK2, TOIE2);              // enable Timer2 Interrupt

}

//******************************************************************
// timer2 setup
void Setup_timer2() {
  // set prscaler to 1, PWM mode to phase correct PWM,  16000000/510 = 31372.55 Hz clock
  TCCR2A = (1 << COM2A1) | (0 << COM2A0) | ( 1 << COM2B1) | ( 0 << COM2B0) | ( 0 << WGM21) | ( 1 << WGM20);
  // Timer2 Clock Prescaler to : 1 =>  16000000/510 = 31372.55 Hz clock
  TCCR2B = (0 << WGM22) | (0 << CS22) | ( 0 << CS21) | ( 1 << CS20);
}


//******************************************************************
// Timer2 Interrupt Service at 31372,550 KHz = 32uSec
// this is the timebase REFCLOCK for the DDS generator
// FOUT = (M (REFCLK)) / (2 exp 32)
ISR(TIMER2_OVF_vect) {
  // dauer ca. 5us  mit sbi... / ein kanal //

  //sbi(PORTB,5);//==digitalWrite(LED,1);
  // for SIN1
  phaccu_a = phaccu_a + tword_a; // soft DDS, phase accu with 32 bits
  icnt_a = phaccu_a >> 24;   // use upper 8 bits for phase accu as frequency information
  // read value fron ROM sine table and send to PWM DAC
  OCR2A = pgm_read_byte_near(sine256 + icnt_a);

  if (OCR2A < 0x80) {  // output digital by PWM info
    cbi(PORTD, 7);
  } else {
    sbi(PORTD, 7);
  }
}
