/* Name: main.c
 * Author: Michael de Silva <michael@mwdesilva.com>
 * Copyright: MIT License
 */

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define F_CPU 8000000UL         // [> Clock Frequency = 8Mhz <]
#define TIMER_PRESCALE 256

/*
 * 7-Segment Character Maps
 *          _____
 *       f / a  / b
 *        /____/
 *     e / g  / c
 *      /____/ o dp
 *       d
 */
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

void tcnt1_delay(int target_freq);
void hundreth_second_timer(void);
void second_timer(void);

/*
 * Refresh frequency = 60 Hz = 16.67 ms period.
 * 16.67/4 = 4.1675 ms per digit.
 */

volatile int16_t val;

ISR(TIMER1_OVF_vect)
{
  /*
   * Enable to increment count on timer overflow @ 2^16 (16-bit timer)
   * Timer Resolution = prescale/input frequency
   * 8/8MHz * (2^16) =    0.066s
   * 64/8MHz * (2^16) =   0.524s
   * 256/8MHz * (2^16) =  2.097s
   * 1024/8MHz * (2^16) = 8.389s
   */

  /*val++;*/
}

/*
 * Do all the startup-time peripheral initializations.
 */
static void ioinit(void){
  /* Setup i/o */
  DDRD = 0xff;                  // set as output i/o
  PORTD = 0x00;
  DDRB = 0xff;                  // set as output i/o
  PORTB = ZERO;

  /* Setup interrupts */
  TIMSK |= (1 << TOIE1);        // Enable overflow interrupt

  switch(TIMER_PRESCALE){
    case 1:
      TCCR1B |= (1 << CS10);                  // no prescaling
      break;
    case 64:
      TCCR1B |= (1 << CS10) | (1 << CS11);    // F_CPU/64
      break;
    case 256:
      TCCR1B |= (1 << CS12);                  // F_CPU/256
      break;
    case 1024:
      TCCR1B |= (1 << CS10) | (1 << CS12);    // F_CPU/1024
      break;
  }

  TCNT1 = 0x00;                 // Clear counter register
  sei();                        // Enable global interrupts
}

int main(void)
{
  val = 0;
  ioinit();                     // setup bootup initialisations
  for(;;){
    /* insert your main loop code here */
    draw_display(val);
    hundreth_second_timer();

    // prevent overflow for completeness sake!
    if (val >= 32767){
      val = 0;
    }
  }
  return 0;                     /* never reached */
}

void tcnt1_delay(int target_freq){
  /*
   * NOTE 3 Hz delay @ 1024 prescaler
   * target_freq = 3;
   * target_count = (8000000/(1024*target_freq)) - 1;  // 2603
   */
  uint16_t target_count = (F_CPU/(TIMER_PRESCALE*target_freq)) - 1;
  if(TCNT1 >= target_count){
    val++;
    TCNT1 = 0;
  }
}

void hundreth_second_timer(void){
  tcnt1_delay(100);        // 100 Hz delay (T = 0.1s) @ 256 prescaler
}

void second_timer(void){
  /* 1 Hz delay @ 256 prescaler */
  tcnt1_delay(1);
}


void draw_display(int value){
  PORTD = 0x08;                 // start with the LS-digit (4th digit).

  /*
   * Extract each digit from the value and set it from the least-significant
   * digit to the most-significant digit onto the 7-segment display.
   */
  while (value > 0) {
    int digit = value % 10;
    // do something with digit
    persist_digit(get_digit(digit));
    PORTD = PORTD >> 1;
    value /= 10;
  }

  // Display leading zero
  if (PORTD == 0x02 || PORTD == 0x01){
    persist_digit(get_digit(0));
    if (val/1000 <= 0){
      PORTD = 0x01;
      persist_digit(get_digit(0));
    }
  }
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
  _delay_ms(4);
  _delay_us(167);
}

