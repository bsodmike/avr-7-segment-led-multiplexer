/* Name: main.c
 * Author: <insert your name here>
 * Copyright: <insert your copyright message here>
 * License: <insert your license reference here>
 */

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 8000000UL         // [> Clock Frequency = 8Mhz <]
#define ZERO 0xC0
#define ONE 0xF9
#define TWO 0xA4
#define THREE 0xB0
#define FOUR 0x99
#define FIVE 0x92
#define SIX 0x82
#define SEVEN 0xF8
#define EIGHT 0x80
#define NINE 0x90

void draw_display(int value);
int get_digit(int num);
void persist_digit(char digit);

/*
 * Refresh frequency = 60 Hz = 16.67 ms
 * 5% duty-cycle (on-time) = 0.8335 ms
 * 833.5/4 = 208.5 us per digit.
 * 95% duty-cycle (off-time) = 15.837 ms
 */

int main(void)
{
  /* insert your hardware initialization here */
  DDRD = 0xff;                  // set as output i/o
  PORTD = 0x00;
  DDRB = 0xff;                  // set as output i/o
  PORTB = 0x00;                 // ... as low

  int i;
  int val = 0;
  for(;;){
    /* insert your main loop code here */
    draw_display(val);
    val++;
  }
  return 0;                     /* never reached */
}

void draw_display(int value){
  PORTD = 0x08;
  while (value > 0) {
    int digit = value % 10;
    // do something with digit
    persist_digit(get_digit(digit));
    PORTD = PORTD >> 1;
    value /= 10;
  }

  /*_delay_ms(13);*/
  /*_delay_us(336);      // total = 15.832 us*/
}

int get_digit(int num){
  int output=0;
  switch(num){
    case 0:
      output = ZERO;
      break;
    case 1:
      output = ONE;
      break;
    case 2:
      output = TWO;
      break;
    case 3:
      output = THREE;
      break;
    case 4:
      output = FOUR;
      break;
    case 5:
      output = FIVE;
      break;
    case 6:
      output = SIX;
      break;
    case 7:
      output = SEVEN;
      break;
    case 8:
      output = EIGHT;
      break;
    case 9:
      output = NINE;
      break;
  }
  return(output);
}

void persist_digit(char value){
  PORTB = value;
  _delay_us(416);
  _delay_us(416);      // total = 832 us

  /*int i;*/
  /*for(i=0;i<200;i++){*/
    /*PORTB = value;*/
    /*_delay_us(1);*/
  /*}*/
}


void test(void){
    /*_delay_ms(1);             // [> max is 262.14 ms / F_CPU in MHz <]*/
    /*[>PORTB ^= 1 << 4;        // [> toggle the LED <]<]*/
    /*PORTD = PORTD << 1;*/
    /*if (PORTD == 0x10) {*/
      /*PORTD = 0x01;*/
    /*}*/
    /*PORTB = TWO;*/
}
