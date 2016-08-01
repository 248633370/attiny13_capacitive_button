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
char atx_status = 0;
char half_light = 0;
volatile char touch_cycle_count;
volatile char touch_cycle_start;
char double_touch;
volatile uint16_t retval;
volatile uint8_t adc_current;
volatile uint8_t adc_sample;
volatile uint8_t adc_count;
/*
 * touch_cycle[] header:
 * 1. touch_cycle start
 * 2. touch_cycle timer count
 * 3. touch_cycle positive count
 * 4. touch_cycle negative count
 */

void pwm_setup(void)
{
	// PWM out on PB1
	TCCR0A |= (1<<COM0B1);
	TCCR0A |= (1<<WGM01)|(1<<WGM00);
	TCCR0B |= (1<<CS01)|(1<<CS00);
	// COM0B1 - 1 set "Clear OC0B on Compare Match, set OC0B at TOP"
	// WGM02:0 - 0b011 set Fast PWM Mode with TOP in 0xFF
	// CS02:0 - 0b011 set clock source divider /64
	// Fast PWM timer 1 cycle = 0,027 sec
	TIMSK0 |= (1<<TOIE0);
	//TOIE0: Timer/Counter0 Overflow Interrupt Enable
	//PB1 output mode
	DDRB |= (1<<PB1);
}

void adc_setup(void)
{
	// config ADC
	ADMUX |= (1<<ADLAR)|(1<<MUX0)|(1<<REFS0);
	// REFS0 - 0 set Vcc as analog reference, 1 set internal ref 1.1V
	// ADLAR - 1 left align, use ADCH register
	// MUX1:0 - 0b01 ADC1 (PB2)

	ADCSRA |= (1<<ADPS2)|(1<<ADPS0);
	ADCSRA |= (1<<ADEN)|(1<<ADSC)|(1<<ADIE);
	// ADPS2:0 - set ADC freq divider 0b101 - /32
	// ADEN - 1 enable ADC
	// ADATE - 1 enable Freerun mode
	// ADIE - 1 ADC Interrupt Enable
	// ADIF - ADC Interrupt Flag (1 mean conversion done)

	//MCUCR |= (1<<SE)|(1<<SM1);
	// SE - 1 Sleep enable
	// SM1:0 - 0b01 ADC Noise Reduction
}

ISR(TIM0_OVF_vect)
{
	if (touch_cycle_start == 1) //if cycle start - accumulate cycle count
	{
		if (touch_cycle_count == 255 ) //prevent overflow
		{
			touch_cycle_count = 0;
		}
		touch_cycle_count++;
	}
}

ISR(ADC_vect)
{
	adc_count++;
	retval += ADCH;
	PORTB |= (1<<PB2); // enable Pull-Up
	static char i;
	for (i=0; i<250; i++){} // some delay
	PORTB &= ~(1<<PB2); // disable Pull-Up
	ADCSRA |= (1<<ADSC); // start ADC conversion
}

void pson_switch()
{
	if (atx_status == 1)
	{
		// if ATX bit on - enable power
		DDRB |= (1<<PB0);
		//Check status vars, set outputs
		if (half_light == 1)
		{
			OCR0B = 0x80; //half duty
		}
		else
		{
			OCR0B = 0xff;
		}
	}
	else
	{
		DDRB &= ~(1<<PB0);	// disable PB0 (ATX PS_ON )
		OCR0B = 0x00;		// disable duty PWM on PB1 (+12V power key)
	}
}

int main(void)
{
	pwm_setup();
	adc_setup();
	sei();	// enable global interrupts
	//sleep_cpu(); // going to sleep

	while (adc_sample == 0)
	{
		if (adc_count >= 4) //take average for simple prevent mistakes
		{
			adc_sample = retval/adc_count;
			adc_count = 0;
			retval = 0;
		}
	}

	//main loop
	while (1)
	{
		if (adc_count >= 4) //take average for simple prevent mistakes
		{
			adc_current = retval/adc_count;
			adc_count = 0;
			retval = 0;
			//about 1,5..2sec capacitor is charge and it was easy than coding ADC checks
			if (adc_sample < adc_current/2)
			{
				if (touch_cycle_start == 0) // enable cycle start flag
				{
					touch_cycle_start = 1;
					touch_cycle_count = 0;
				}
				if (touch_cycle_start == 1  && touch_cycle_count > 16 && touch_cycle_count < 32)
				{
					double_touch = 1; // if second touch between 0,4...0,8 sec, then half_light
				}
			}
		}
		if (touch_cycle_count >= 40)
		{
			if (atx_status == 0)
			{
				if (double_touch == 1)
				{
					half_light = 1;
				}
				atx_status = 1;
			}
			else
			{
				if ((half_light == 0 && double_touch == 1) )
				{
					atx_status = 1;
					half_light = 1;
				}
				else
				{
					atx_status = 0;
					half_light = 0;
				}
			}
			touch_cycle_start = 0; // 1 secod - stop touch cycle
			double_touch = 0;	// disable vars
			touch_cycle_count = 0;
		}
		if (touch_cycle_start == 0)
		{
			pson_switch();	// Apply the changes to outputs only if touch cycle is end
		}
	}
	// If cycle is end - something wrong
}
