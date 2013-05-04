/* Name: main.c
 * Author: Michael de Silva <michael@mwdesilva.com>
 * Copyright: MIT License
 */

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define F_CPU 8000000UL         // [> Clock Frequency = 8Mhz <]
/*#define TIMER0_PRESCALE 256*/
#define TIMER1_PRESCALE 256

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

void tcnt1_delay(uint16_t target_freq);
void hundred_hertz_timer(void);
void second_timer(void);
void set_TIMER1_prescale(uint16_t prescale);

/*
 * Refresh frequency = 60 Hz = 16.67 ms period.
 * 16.67/4 = 4.1675 ms per digit.
 */

volatile int16_t val = 0;

// 8-bit timer ISR
ISR(TIMER0_OVF_vect){
  /*
   * 1/8MHz * (2^8) =    32us
   * 8/8MHz * (2^8) =    256us
   * 64/8MHz * (2^8) =   2.048ms
   * 256/8MHz * (2^8) =  8.192ms
   * 1024/8MHz * (2^8) = 32.768ms
   */
  draw_display(val);
  TCNT0 = 0;
}

// 16-bit timer ISR
ISR(TIMER1_OVF_vect){
  /*
   * Enable to increment count on timer overflow @ 2^16 (16-bit timer)
   * Timer Resolution = prescale/input frequency
   * 1/8MHz * (2^16) =    8.192ms
   * 8/8MHz * (2^16) =    65.536ms
   * 64/8MHz * (2^16) =   524.288ms
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
  set_TIMER1_prescale(TIMER1_PRESCALE);
  TCNT1 = 0x00;                 // Clear counter register

  // TCCR0
  TIMSK |= (1 << TOIE0);        // Enable overflow interrupt
  TCCR0 |= (1 << CS02);         // F_CPU/256
  TCNT0 = 0x00;                 // Clear counter register

  sei();                        // Enable global interrupts
}

int main(void)
{
  val = 0;
  ioinit();                     // setup bootup initialisations
  for(;;){
    /* insert your main loop code here */
    hundred_hertz_timer();

    // prevent overflow for completeness sake!
    if (val >= 32767){
      val = 0;
    }
  }
  return 0;                     /* never reached */
}

void tcnt1_delay(uint16_t target_freq){
  /*
   * NOTE 3 Hz delay @ 1024 prescaler
   * target_freq = 3;
   * target_count = (8000000/(1024*target_freq)) - 1;  // 2603
   */
  uint16_t target_count = (F_CPU/(TIMER1_PRESCALE*target_freq)) - 1;
  if(TCNT1 >= target_count){
    val++;
    TCNT1 = 0;
  }
}

void hundred_hertz_timer(void){
  tcnt1_delay(100);             // 100 Hz delay (T = 0.1s)
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

  // Clear leading digit of ghosting without the delay in persist_digit()
  if (PORTD == 0x02 || PORTD == 0x01){
    PORTB = 0xff;
    if (val/1000 <= 0){
      PORTD = 0x01;
      PORTB = 0xff;
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
  _delay_us(409);               // 5% of 8.192ms TIMER0 interrupt.
}

void set_TIMER1_prescale(uint16_t prescale){
  switch(prescale){
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
}
