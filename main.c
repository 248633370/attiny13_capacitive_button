// Nechaev Anton 20.10.2015
// Capacitive button on Attiny13/ATtiny85 for led lamp
//
// DDRxn=0 PORTxn=0 – Режим: HI-Z – высоко импендансный вход.
// DDRxn=0 PORTxn=1 – Режим: PullUp – вход с подтяжкой до лог.1.
// DDRxn=1 PORTxn=0 – Режим: Выход – на выходе лог.0. (почти GND)
// DDRxn=1 PORTxn=1 – Режим: Выход – на выходе лог.1. (почти VCC)

#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>

int main(void)
{
	//vars for set full and half light
	//bool light_12v = false;
	//bool light_5v = false;

	//set PB0 and PB2 in output mode
	DDRB |= (1<<PB0)|(1<<PB2);
	DDRB &= ~(1<<PB4);	//PB4 as input

	// config PWM out on PB0
	TCCR0A |= (1<<COM0A1)|(1<<WGM01)|(1<<WGM00) ;
	TCCR0B |= (1<<WGM02)|(1<<CS01)|(1<<CS00);
	// COM0A1 - 1 set "Clear OC0A on Compare Match, set OC0A at TOP"
	// WGM02:0 - 111 set Fast PWM Mode with TOP in OCRA
	//CS02:0 - 011 set clock source divider /64

	// config ADC
	ADMUX |= (1<<REFS0)|(1<<ADLAR);
	// REFS0 - 1 set internal ref
	// ADLAR - 1 left align, use ADCH register
	ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1);
	// ADEN - 1 enable ADC
	// ADPS2:0 - set freq divider 0b110 - /64

	//main loop
	while (1)
	{
		if (PINB & (1<<PB4)) //if not null value => true
		{
			PORTB ^= (1<<PB0);
			_delay_ms(300);
//			PORTB |= (1<<PB0);}
//		else {
//			PORTB &= ~(1<<PB0);    //Turns OFF LED
		}
	}
	// If cycle is end - something wrong
	return 1;
}
