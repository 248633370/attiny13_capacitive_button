// Nechaev Anton 20.10.2015
// Capacitive button on ATtiny13/ATtiny85 for led lamp, power from atx power supply
//
// DDRxn=0 PORTxn=0 – Режим: HI-Z – высоко импендансный вход.
// DDRxn=0 PORTxn=1 – Режим: PullUp – вход с подтяжкой до лог.1.
// DDRxn=1 PORTxn=0 – Режим: OutLow Выход – на выходе лог.0. (почти GND)
// DDRxn=1 PORTxn=1 – Режим: OutHi Выход – на выходе лог.1. (почти VCC)

/*
 * # Pinout schema
 * PB0 - ATX PSON
 * PB1 - PWM for MOS FET led light
 * PB2(ADC1) - capacitive sensor with RC-current
 *
 * # Adc capacitive touch algorithm
 * Pull-Up -> On
 * ADMUX -> PB2(ADC1) to GND
 * Pull-Up -> Off
 * ADMUX -> PB2(ADC1) to ADC
 * Measure
 */

#define F_CPU 4800000

#include <avr/io.h>
#include <avr/interrupt.h>
//#include <avr/sleep.h>

// vars for set power on, full and half light
volatile char atx_status = 0;
volatile char half_light = 0;
volatile uint16_t retval;
volatile uint8_t adc_current;
volatile uint8_t adc_sample;
volatile uint8_t adc_count;

void pwm_setup(void)
{
	//PB0 - Pull-UP on (We can disable pull-up globally using PUD in MCUCR)
	PORTB |= (1<<PB0);
	//PB1 in output mode
	DDRB |= (1<<PB1);
	// PWM out on PB1
	TCCR0A |= (1<<WGM01)|(1<<WGM00);
	TCCR0A |= (1<<COM0B1);
	TCCR0B |= (1<<CS00);
	// COM0B1 - 1 set "Clear OC0B on Compare Match, set OC0B at TOP"
	// WGM02:0 - 0b011 set Fast PWM Mode with TOP in OCRA
	// CS02:0 - 0b001 set clock source divider /32

	//OCR0A = 0x80; // Duty about 50% (128 from 255)
	//OCR0B = 0x80; // Duty about 50% (128 from 255)
}

void adc_setup(void)
{
	// config ADC
	ADMUX |= (1<<ADLAR);
	ADMUX |= (1<<MUX0);
	// REFS0 - 0 set Vcc as analog reference
	// ADLAR - 1 left align, use ADCH register
	// MUX1:0 - 0b01 ADC1 (PB2)

	ADCSRA |= (1<<ADPS2)|(1<<ADPS0);
	ADCSRA |= (1<<ADEN)|(1<<ADSC)|(1<<ADIE);
	// ADPS2:0 - set freq divider 0b101 - /32
	// ADEN - 1 enable ADC
	// ADATE - 1 enable Freerun mode
	// ADIE - 1 ADC Interrupt Enable
	// ADIF - ADC Interrupt Flag (1 mean conversion done)

	//MCUCR |= (1<<SE)|(1<<SM1);
	// SE - 1 Sleep enable
	// SM1:0 - 0b01 ADC Noise Reduction
}

/*
	unsigned int adc_read(void)
	{
		ADCSRA |= (1<<ADSC); //start conversion
		while(!(ADCSRA & (1<<ADIF))); //wait for conversion to finish
		ADCSRA |= (1<<ADIF); //reset the flag
		return ADC; //return value
	}
*/

ISR(ADC_vect)
{
	adc_count++;
	retval += ADCH;
	PORTB |= (1<<PB2); // enable Pull-Up
	ADMUX |= (1<<MUX1); // switch multiplexer to PB3(ADC3)
	static char i;
	for (i=0; i<250; i++){} // some delay
	PORTB &= ~(1<<PB2); // disable Pull-Up
	ADMUX &= ~(1<<MUX1); // switch multiplexer to PB2(ADC1)
	ADCSRA |= (1<<ADSC); // start ADC conversion
}

void pson_switch()
{
	if (atx_status = 1)
	{
		// if ATX on - disable power
		DDRB &= ~(1<<PB0);
		// reset PWM to "0"
		OCR0B = 0x00;
		//Reset status vars
		atx_status = 0;
		if (half_light == 1)
			{
			half_light = 0;
			}
	}
	else
	{
		DDRB |= (1<<PB0);	// enable PB0 (ATX PS_ON )
		OCR0B = 0x00;		// enable duty PWM on PB1 (+12V power key)
		atx_status = 1;
		half_light = 1;
	}
}

int main(void)
{
	//pwm_setup();
	adc_setup();
	sei();	// enable global interrupts
	//sleep_cpu(); // going to sleep

	while (adc_count < 10){}
	adc_sample = retval/adc_count;
	adc_count = 0;
	retval = 0;

	//main loop
	while (1)
	{
		if (adc_count >= 4)
		{
			adc_current = retval/adc_count;
			adc_count = 0;
			retval = 0;
			if (adc_sample < adc_current)
			{
				pson_switch();
				static char i;
				for (i=0; i<250; i++){}
			}
		}
	}
	// If cycle is end - something wrong
}
