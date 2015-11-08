// Nechaev Anton 20.10.2015
// Capacitive button on Attiny13/ATtiny85 for led lamp
//
// DDRxn=0 PORTxn=0 – Режим: HI-Z – высоко импендансный вход.
// DDRxn=0 PORTxn=1 – Режим: PullUp – вход с подтяжкой до лог.1.
// DDRxn=1 PORTxn=0 – Режим: Выход – на выходе лог.0. (почти GND)
// DDRxn=1 PORTxn=1 – Режим: Выход – на выходе лог.1. (почти VCC)

#define F_CPU 4800000

#include <avr/io.h>
//#include <stdbool.h>
//#include <avr/interrupt.h>

void pwm_setup(void)
{
	// set PB0 and PB1 in output mode
	DDRB |= (1<<PB0)|(1<<PB1);

	// PWM out on PB0
	TCCR0A |= (1<<WGM01)|(1<<WGM00);
	TCCR0A |= (1<<COM0A1);

	//TCCR0B |= (1<<WGM02);
	//TCCR0B |= (1<<CS02)|(1<<CS01)|(1<<CS00);
	TCCR0B |= (1<<CS00);
	// COM0A1 - 1 set "Clear OC0A on Compare Match, set OC0A at TOP"
	// WGM02:0 - 111 set Fast PWM Mode with TOP in OCRA
	// CS02:0 - 011 set clock source divider /64
	OCR0A = 0xF0; // Duty about 50% (128 from 255)
}

void adc_setup(void)
{
	// config ADC
	ADMUX |= (1<<ADLAR);
	ADMUX |= (1<<MUX1);
	// REFS0 - 0 set Vcc as analog reference
	// ADLAR - 1 left align, use ADCH register

	ADCSRA |= (1<<ADPS2)|(1<<ADPS0);
	ADCSRA |= (1<<ADEN)|(1<<ADSC)|(1<<ADATE);
	// ADPS2:0 - set freq divider 0b101 - /32
	// ADEN - 1 enable ADC
	// ADATE - 1 enable Freerun mode
}


int main(void)
{
	// vars for set full and half light
	//bool light_12v = false;
	//bool light_5v = false;

	pwm_setup();
	adc_setup();

	int adc_current;
	int adc_prev;

	//main loop
	while (1)
	{
		if ( ADSC )
		{
			adc_prev = adc_current;
			adc_current = ADCH;
			ADCSRA |= (1<<ADSC);
		}
	}
	// If cycle is end - something wrong
	return 1;
}
