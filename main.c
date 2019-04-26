/*
 * main.c
 *
 * Created: 4/4/2019 1:37:23 PM
 * Author : Sterling
 */ 

#define F_CPU	16000000UL			// CPU speed
#define BAUD	9600				// baud-rate
#define MYBURR	F_CPU/16/BAUD - 1	// value to initialize for baud-rate

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>	

extern void ZERO();
extern void ONE();

// Initialize ports
void PORT_Init(void);

// Analog to Digital Converter initialization
void ADC_Init(void);

// Analog to Digital read-in
uint8_t ADC_read(uint8_t pin);

// Universal Asynchronous communication initialization
void USART_Init(unsigned int ubrr);

// Universal Asynchronous communication transmission
void USART_Transmit(unsigned char data);

// Universal Asynchronous communication receiving
unsigned char USART_Receive(void);

// Universal Asynchronous communication transmitting whole strings
void USART_String(char *word);

// Convert a value to a string
char *to_string(uint8_t number, char *buffer);

// Color will be shown on LED from axis values
void color_convert(uint8_t *axis_array);

// Convert readouts from axis to actual color
void color(uint8_t value);

// Delay function (no idea on length, just estimate)
void delay(void);

// initialize array for axis data and buffer
uint8_t axis_array[3] = {0, 0, 0};
char buffer[4] = "";

int main(void)
{
	// initialize serial connection and analog to digital converter
	PORT_Init();
	USART_Init(MYBURR);
	ADC_Init();
	
	// Start timer using 1024 prescaler
	TCCR0B = (1 << CS02);// | (1 << CS00);
	// Set counter initial value to 0
	TCNT0 = 0x00;
	// Timer Interrupt Enabled (8-bit)
	TIMSK0 = (1 << TOIE0);
	
	// set global interrupt
	sei();
		
	while(1);
	
	return 0;
}

// interrupt service routine
ISR(TIMER0_OVF_vect) {
	// z-axis
	axis_array[0] = ADC_read(0);
	// y-axis
	axis_array[1] = ADC_read(1);
	// x-axis
	axis_array[2] = ADC_read(2);

	// transmit data to serial console
	USART_String("\n\r");
	USART_String(to_string(axis_array[2], buffer));
	USART_Transmit(',');
	USART_String(to_string(axis_array[1], buffer));
	USART_Transmit(',');
	USART_String(to_string(axis_array[0], buffer));
			
	// run color conversion from sensor data
	color_convert(axis_array);
}

// take each axis and convert it to a single color
void color_convert(uint8_t *axis_array) {
	// x-axis (green)
	color(*axis_array);
	axis_array++;
	// y-axis (red)
	color(*axis_array);
	axis_array++;
	// z-axis (blue)
	color(*axis_array);
}

// take an 8-bit number and call corresponding
// cycles for neopixel
void color(uint8_t value) {
	uint8_t midpoint = 128;	
	
	// if no force, then eliminate the color completely
	// (128 -> 0)
	value = value % midpoint;
	// dim the brightness of the neopixel
	value &= 0xF0;
	
	// for every bit, call a command
	for(int i = 0; i < 8; i++) {
		if(value & (1 << i)) {
			ONE();
		} else {
			ZERO();
		}
	}
}

void PORT_Init(void) {
	// PortA is input (acceleromter)
	DDRA = 0x00;
	// PORTB is output (neopixel)
	DDRB = 0xFF;
}

// initialize the analog to digital converter from PORTA
void ADC_Init(void) {
	
	// * Analog to Digital Converter Status Register A *
	// Pre-scaler of 64 -> 16MHz/64 = 250KHz
	// ADEN -> enable analog to digital conversion
	// ADSC -> start the conversion from digital to analog
	// ADIF -> interrupt flag
	// ADIE -> ADC interrupt enable
	
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 0x07 (pre-scaler 16M/128)
	ADCSRA |= (1 << ADEN) | (1 << ADIE);				  // 0x8F

	// * Multiplexer Selection Register *
	// ADLAR -> ADC Left Adjust Result (high 8-bits)
	// First three bits in ADC to be read (X,Y,Z)
	ADMUX |= (1 << ADLAR);   //0x20
}

// read from a single port and convert to a digital
// value from a signal value
uint8_t ADC_read(uint8_t pin) {
	// axis x = 2
	// axis y = 1
	// axis z = 0
	
	
	// select the corresponding pins from PORTA 0~7
	pin &= 0x07;						// restrict to pins 0-7 (error check)
	ADMUX = (ADMUX & 0xE0) | pin;		// clears the bottom 5 bits before selecting pin
	
	// start analog to digital conversion
	ADCSRA |= (1<<ADSC);
	
		// wait for conversion to complete
	// ADSC becomes ’0' again
	// till then, run loop continuously
	while(ADCSRA & (1<<ADSC)) {
		continue;
	}
	
	delay();
	// return value (high-bit, left-justified)
	return (ADCH);
}

// initialize serial connection
void USART_Init(unsigned int ubrr) {
	// Place corresponding value for baud-rate  into UBRR0
	UBRR0H = (unsigned char) (ubrr>>8);
	UBRR0L = (unsigned char) (ubrr);
	// Enable receiver and transmitter
	UCSR0B = (1 << TXEN0) | (1 << TXEN0);
	// Enable 8 data bits and 2 stop bits
	UCSR0C = (1 << USBS0) | (3 << UCSZ00);
}

// receive single character from computer console
unsigned char USART_Receive(void) {
	while ( !(UCSR0A & (1 << RXC0)) ) {
		continue;
	}
	
	return UDRE0;
}

// transmit a single character
void USART_Transmit(unsigned char data) {
	// while register is empty for char is empty
	while( !(UCSR0A & (1 << UDRE0)) ) {
		continue;
	}
	
	// return character
	UDR0 = data;
}

// transmit a string rather than a single character
void USART_String(char *word) {
	// iterate through string transmitting until string is empty
	for(int i = 0; word[i] != '\0'; i++) {
		USART_Transmit(word[i]);
	}
}

// convert string using the utoa function
char *to_string(uint8_t number, char *buffer) {
	// uses base-10
	utoa(number, buffer, 10);

	return buffer;
}

void delay(void) {
	
}