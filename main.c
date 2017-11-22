/*
    Author Dillon Dickerson
    Class SER 456
    Semster Project
*/
#define EN PD1
#define RS PD0
#define LCD_PORT PORTB
#define CTRL_PORT PORTD
#define F_CPU 1000000       // 1 MHz i think
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

void init_AVR_pins(void) {
    DDRB = 0xff;
    DDRD = 0x03;
}

void init_LCD(void) {
    CTRL_PORT &= ~(1 << RS);
    CTRL_PORT &= ~(1 << EN);
    _delay_ms(200);
    LCD_PORT = 0x30;
    double_toggle_enable();
    _delay_ms(20);
    double_toggle_enable();
    _delay_ms(20);
    double_toggle_enable();
    _delay_ms(20);
    LCD_PORT = 0b00111100;              // Set Interface Length
    double_toggle_enable();
    clear_LCD();
    // Enable Cursor (7th bit)
    LCD_PORT = 0b00001100;              // Cursor On (BLINK is 8th bit)
    double_toggle_enable();
}

void init_analog(void)
{
    ADMUX |= (1 << REFS0);
    ADCSRA |= (1 << ADPS0) | (1 << ADPS1);              // 1/8 prescaler
    ADCSRA |= (1 << ADEN);
}

void double_toggle_enable(void) {
    CTRL_PORT |= (1 << EN);
    _delay_ms(5);
    CTRL_PORT &= ~(1 << EN);
    _delay_ms(5);
    CTRL_PORT |= (1 << EN);
    _delay_ms(5);
}

void write_char_to_LCD(char letter) {
    LCD_PORT = letter;
    CTRL_PORT |= (1 << RS);
    double_toggle_enable();
    LCD_PORT = 0x00;
    CTRL_PORT &= ~(1 << RS);
}

void reset_cursor_pos(void) {
    LCD_PORT = 0b00000010;
    CTRL_PORT &= ~(1 << RS);
    double_toggle_enable();
}

void clear_LCD(void) {
    LCD_PORT = 0b00000001;              // Clear Display
    CTRL_PORT &= ~(1 << RS);
    double_toggle_enable();
}

uint16_t read_analog(void) {
	uint16_t tmpVal = 0x0000;
	ADCSRA |= (1 << ADSC);
    loop_until_bit_is_clear(ADCSRA, ADSC);
    tmpVal = ADC;
	return tmpVal;
}

void display_analog_binary(uint16_t tmpVal) {
	//clear_LCD();
	reset_cursor_pos();
    uint16_t tmp = tmpVal / 0b00000101;
    int i = 9;
    while (i >= 0) {
        // write character
		if (1 == ((tmp >> i) & 0x01)) {
			// if bit is 1
			write_char_to_LCD('1');
		} else {
			// if bit is 0
			write_char_to_LCD('0');
		}
        i--;
    }
}

void display_analog_decimal(uint16_t tmpVal) {
    reset_cursor_pos();
    //tmpVal = tmpVal - 0b00110111;
    int d = 5;
    tmpVal *= 10;
    uint16_t tmp = tmpVal / 0b00000101;
    // plus 55
    //tmp = tmp - (0b00110111);
    char temperature[3];
    itoa(tmp, temperature, 10);
    if (tmp < 100) {
        /*temperature[3] = temperature[2];
        temperature[2] = temperature[1];
        temperature[1] = temperature[0];
        temperature[0] = '0';*/
        //temperature[3] = temperature[2];
        temperature[2] = temperature[1];
        temperature[1] = temperature[0];
        temperature[0] = '0';
    }
    int i = 0;
    while (i < 3) {
        // write character
        if (i == 2) {
            write_char_to_LCD('.');
        }
		write_char_to_LCD(temperature[i]);
        i++;
    }
    write_char_to_LCD(' ');
    write_char_to_LCD(0b11011111);
    write_char_to_LCD('C');
}

void write_temperature_label(void) {
    clear_LCD();
    // move Cursor to second line
    CTRL_PORT &= ~(1 << RS);
    LCD_PORT = 0b11000000;
    double_toggle_enable();
    //
    write_char_to_LCD('T');
    write_char_to_LCD('e');
    write_char_to_LCD('m');
    write_char_to_LCD('p');
    write_char_to_LCD('e');
    write_char_to_LCD('r');
    write_char_to_LCD('a');
    write_char_to_LCD('t');
    write_char_to_LCD('u');
    write_char_to_LCD('r');
    write_char_to_LCD('e');
    write_char_to_LCD(' ');
    write_char_to_LCD('(');
    write_char_to_LCD(0b11011111);
    write_char_to_LCD('C');
    write_char_to_LCD(')');
}

int main(void){
    //_delay_ms(5000);
	uint16_t ADCval = 0xffff;
    init_AVR_pins();
    init_LCD();
	init_analog();
    write_temperature_label();
    //reset_cursor_pos();
    while (1) {
        //clear_LCD();
        reset_cursor_pos();
        //write_char_to_LCD('A');
        //write_char_to_LCD('B');
        //write_char_to_LCD('C');
        _delay_ms(500);
        //double_toggle_enable();
		ADCval = read_analog();
		display_analog_decimal(ADCval);
        //display_analog_binary(ADCval);
    }
    return 0;
}
