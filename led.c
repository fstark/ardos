#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>


void w1()
{
  _delay_ms(1);
}

void wn( int n )
{
    w1();
    while (n--)
        w1();
}

int main (void)
{
 /* set pin 5 of PORTB for output*/
 DDRB |= _BV(DDB5);
 
 int delay = 1000;

 while(1)
 {
  /* set pin 5 high to turn led on */
  PORTB |= _BV(PORTB5);
  wn(delay);
 
  /* set pin 5 low to turn led off */
  PORTB &= ~_BV(PORTB5);
  wn(delay);

  if (delay>100)
      delay = delay * 0.5;
 }
}
