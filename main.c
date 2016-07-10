// Nechaev Anton 20.10.2015
// Capacitive button on ATtiny13/ATtiny85 for led lamp, power from atx power supply
//
// DDRxn=0 PORTxn=0 – Режим: HI-Z – высоко импендансный вход.
// DDRxn=0 PORTxn=1 – Режим: PullUp – вход с подтяжкой до лог.1.
// DDRxn=1 PORTxn=0 – Режим: Выход – на выходе лог.0. (почти GND)
// DDRxn=1 PORTxn=1 – Режим: Выход – на выходе лог.1. (почти VCC)

#define F_CPU 4800000

#include <avr/io.h>
#include <stdbool.h>
#include <avr/interrupt.h>

// vars for set power on, full and half light
bool atx_status = false;
bool full_light = false;

// write digital "high" to pin <pn> on port <prt>
//#define DIGIWRITE_H(prt, pn) prt |= (1<<pn)
//
// write digital "low" to pin <pn> on port <prt>
//#define DIGIWRITE_L(prt, pn) prt &= ~(1<<pn)

void pwm_setup(void)
{
	// set PB0 and PB1 in output mode
	DDRB |= (1<<PB1)|(1<<PB0);

	// PWM out on PB0
	TCCR0A |= (1<<WGM01)|(1<<WGM00);
	TCCR0A |= (1<<COM0A1);
	TCCR0A |= (1<<COM0B1);
	TCCR0B |= (1<<CS00);
	// COM0A1 - 1 set "Clear OC0A on Compare Match, set OC0A at TOP"
	// WGM02:0 - 0b011 set Fast PWM Mode with TOP in OCRA
	// CS02:0 - 0b001 set clock source divider /32

	//OCR0B = 0x80; // Duty about 50% (128 from 255)
}


void adc_setup(void)
{
	// config ADC
	ADMUX |= (1<<ADLAR);
	ADMUX |= (1<<MUX1);
	// REFS0 - 0 set Vcc as analog reference
	// ADLAR - 1 left align, use ADCH register
	// MUX1:0 - 0b10 ADC2 (PB4)

	ADCSRA |= (1<<ADPS2)|(1<<ADPS0);
	ADCSRA |= (1<<ADEN)|(1<<ADSC)|(1<<ADATE)|(1<<ADIE);
	// ADPS2:0 - set freq divider 0b101 - /32
	// ADEN - 1 enable ADC
	// ADATE - 1 enable Freerun mode
	// ADIE - 1 enable ADC interrupt
	// ADIF - 1 enable
}

void pson_switch()
{
	if (atx_status)
	{
		DDRB &= ~(1<<PB2);
		atx_status = false;
		OCR0B = 0x00;
		full_light = false;
	}
	else
	{
		DDRB |= (1<<PB2);	// enable PB2 (ATX PS_ON )
		OCR0B = 0x80;		// enable half duty PWM on PB1 (+12V power key)
	}
}


int main(void)
{
	//pwm_setup();
	adc_setup();
	sei();	// enable global interrupts

	int adc_current;
	int adc_prev;

	//main loop
	while (1)
	{

	}
	// If cycle is end - something wrong
	return 1;
}
