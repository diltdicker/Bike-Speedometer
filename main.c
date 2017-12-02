/*
    Author Dillon Dickerson
    Class SER 456
    Semster Project
*/
#define EN PD1
#define RS PD0
#define LCD_PORT PORTB
#define CTRL_PORT PORTD
#define CTRL_PIN PIND
#define MODE_BUTTON PD6
#define RESET_BUTTON PD7
#define REED_PIN PD2    // REED SENSOR
#define F_CPU 1000000       // 1 MHz i think
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>

void init_Timer(void) {
    TCCR0A |= (1 << WGM01); // CTC ON

    OCR0A = 0x27; // 39 -- interrupt on every 0.01 of a sec

    TIMSK0 |= (1 << OCIE0A); // requires sei();

    TCCR0B |= (1 << CS02); // clk/256
}

// must be called before init_Timer()
void init_Interrupt(void) {

    EIMSK |= (1 << INT0);
    //EICRA |= (1 << ISC00);
    EICRA |= (1 << ISC01);
    sei();
}

void init_AVR_pins(void) {
    DDRB = 0xff;
    DDRD = 0x03;
    CTRL_PORT |= (1 << MODE_BUTTON);        // Pull up resistors
    CTRL_PORT |= (1 << RESET_BUTTON);
    CTRL_PORT |= (1 << REED_PIN);
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
    ADMUX &= ~(1 << ADLAR);     // shift right
    ADCSRA |= (1 << ADSC);
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

void display_time (uint64_t time) {
    time = time / 60;
    uint8_t sec = time % 60;
    time = time / 60;
    uint8_t min = time % (60);
    time = time / 60;
    uint8_t hour = time % (60);

    char hours[2];
    char minutes[2];
    char seconds[2];

    itoa (sec, seconds, 10);
    itoa (min, minutes, 10);
    itoa (hour, hours, 10);

    uint8_t i = 0;
    uint8_t k = 2;

    if (hour < 10){
        write_char_to_LCD('0');
        k = k - 1;
    }
    if (hour == 0) {
        write_char_to_LCD('0');
        k = k - 1;
    }
    while (i < k) {
        write_char_to_LCD(hours[i]);
        i = i + 1;
    }

    write_char_to_LCD(':');
    i = 0;
    k = 2;

    if (min < 10){
        write_char_to_LCD('0');
        k = k - 1;
    }
    if (min == 0) {
        write_char_to_LCD('0');
        k = k - 1;
    }
    while (i < k) {
        write_char_to_LCD(minutes[i]);
        i = i + 1;
    }

    write_char_to_LCD(':');
    i = 0;
    k = 2;

    if (sec < 10){
        write_char_to_LCD('0');
        k = k - 1;
    }
    if (sec == 0) {
        write_char_to_LCD('0');
        k = k - 1;
    }
    while (i < k) {
        write_char_to_LCD(seconds[i]);
        i = i + 1;
    }

    /*char time_10[16];
    reset_cursor_pos();
    itoa(time, time_10, 10);
    uint64_t tmp = 1;
    uint8_t i = 0;
    uint8_t k = 0;
    while (time > tmp ) {
        tmp = tmp * 10;
        k = k + 1;
    }
    while (i < k) {
        // write character
		write_char_to_LCD(time_10[i]);
        i++;
    }*/

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
    //tmpVal = tmpVal - 220;
    tmpVal = tmpVal / 2; // honestly i don't know the proper math cause it keeps on reading wrong
    //tmpVal = tmpVal + 55;

    char temp[3];
    itoa (tmpVal, temp, 10);

    uint8_t i = 0;
    uint8_t k = 0;
    uint16_t j = 1;
    while (tmpVal > j) {
        j = j * 10;
        k = k + 1;
    }

    while (i < k) {
        write_char_to_LCD(temp[i]);
        i = i + 1;
    }

    write_char_to_LCD(0b11011111);
    write_char_to_LCD('C');
    write_char_to_LCD(' ');
}

display_feet(uint64_t feet) {
    reset_cursor_pos();
    char feet_10[16];
    reset_cursor_pos();
    itoa(feet, feet_10, 10);
    uint64_t tmp = 1;
    uint8_t i = 0;
    uint8_t k = 0;
    while (feet > tmp ) {
        tmp = tmp * 10;
        k = k + 1;
    }
    while (i < k) {
        // write character
		write_char_to_LCD(feet_10[i]);
        i++;
    }
}

void display_speed_binary(uint16_t spd_2) {
    reset_cursor_pos();
    int i = 16;
    while (i >= 0) {
        // write character
		if (1 == ((spd_2 >> i) & 0x01)) {
			// if bit is 1
			write_char_to_LCD('1');
		} else {
			// if bit is 0
			write_char_to_LCD('0');
		}
        i--;
    }
}

void display_speed(uint16_t spd) {
    clear_LCD();
    write_speed_label();
    reset_cursor_pos();
    //char speed[4];
    //itoa(spd, speed, 10);
    char speed_10[8];
    itoa(spd, speed_10, 10);
    uint8_t i = 0;
    uint8_t k = 0;
    uint32_t tmp = 1;
    while (spd > tmp) {
        tmp = tmp * 10;
        k = k + 1;
    }
    while (i < k) {
        // write character
		write_char_to_LCD(speed_10[i]);
        i++;
    }

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

void write_milage_label(void) {
    clear_LCD();
    // move Cursor to second line
    CTRL_PORT &= ~(1 << RS);
    LCD_PORT = 0b11000000;
    double_toggle_enable();
    //
    write_char_to_LCD('F');
    write_char_to_LCD('o');
    write_char_to_LCD('o');
    write_char_to_LCD('t');
    write_char_to_LCD('a');
    write_char_to_LCD('g');
    write_char_to_LCD('e');
    write_char_to_LCD(' ');
    write_char_to_LCD(' ');
    write_char_to_LCD('(');
    write_char_to_LCD('f');
    write_char_to_LCD('e');
    write_char_to_LCD('e');
    write_char_to_LCD('t');
    write_char_to_LCD(')');
    write_char_to_LCD(' ');
}

void write_speed_label(void) {
    clear_LCD();
    // move Cursor to second line
    CTRL_PORT &= ~(1 << RS);
    LCD_PORT = 0b11000000;
    double_toggle_enable();
    //
    write_char_to_LCD('S');
    write_char_to_LCD('p');
    write_char_to_LCD('e');
    write_char_to_LCD('e');
    write_char_to_LCD('d');
    write_char_to_LCD('o');
    write_char_to_LCD('m');
    write_char_to_LCD('e');
    write_char_to_LCD('t');
    write_char_to_LCD('e');
    write_char_to_LCD('r');
    write_char_to_LCD(' ');
    write_char_to_LCD('f');
    write_char_to_LCD('/');
    write_char_to_LCD('s');
    write_char_to_LCD(' ');
}

void write_timer_label(void) {
    clear_LCD();
    // move Cursor to second line
    CTRL_PORT &= ~(1 << RS);
    LCD_PORT = 0b11000000;
    double_toggle_enable();
    //
    write_char_to_LCD('T');
    write_char_to_LCD('i');
    write_char_to_LCD('m');
    write_char_to_LCD('e');
    write_char_to_LCD('r');
    write_char_to_LCD(' ');
    write_char_to_LCD('(');
    write_char_to_LCD('H');
    write_char_to_LCD('H');
    write_char_to_LCD(':');
    write_char_to_LCD('M');
    write_char_to_LCD('M');
    write_char_to_LCD(':');
    write_char_to_LCD('S');
    write_char_to_LCD('S');
    write_char_to_LCD(')');
}

// Global Variables
uint8_t flag;
uint64_t total_time;
uint16_t current_speed;
uint64_t feet_taveled; // increments every 52.8 feet e.g. 0.01
uint32_t ticks_per_revolution;
uint32_t tire_size = 628; // 26" tire = 75.31" circumfrence = 6.27.5 ft * 100 = 628
int main(void){
    //_delay_ms(5000);
    flag = 0x00; // for the double touch on the REED SENSOR
    ticks_per_revolution = 0x0000;
    current_speed = 0x0000;
    feet_taveled = 0;
    tire_size = 628;
    total_time = 0;
	uint16_t ADCval = 0xffff;
    uint8_t lcdMode = 0x00; // 0x00 == temperature, 0x01 == Milage, 0x02 == timer
    init_AVR_pins();
    init_LCD();
	init_analog();
    init_Interrupt();
    init_Timer();
    write_temperature_label();
    //reset_cursor_pos();
    while (1) {
        //clear_LCD();
        if (bit_is_clear(CTRL_PIN, MODE_BUTTON)) {
            _delay_ms(5);
            if (bit_is_clear(CTRL_PIN, MODE_BUTTON)) {
                if (lcdMode == 0x00) {
                    lcdMode = 0x01;
                    write_milage_label();
                } else if (lcdMode == 0x01) {
                    lcdMode = 0x02;
                    write_timer_label();
                } else if (lcdMode == 0x02) {
                    lcdMode = 0x03;
                    write_speed_label();
                } else if (lcdMode == 0x03) {
                    lcdMode = 0x00;
                    write_temperature_label();
                }
            }
        }
        if (bit_is_clear(CTRL_PIN, RESET_BUTTON)) {
            _delay_ms(5);
            if (bit_is_clear(CTRL_PIN, RESET_BUTTON)) {
                if (lcdMode == 0x00) {
                    // do nothing
                } else if (lcdMode == 0x01) {
                    // RESET MILAGE
                    feet_taveled = 0;
                    clear_LCD();
                    write_milage_label();
                } else if (lcdMode == 0x02) {
                    // RESET TIMER
                    total_time = 0;
                    clear_LCD();
                    write_timer_label();
                } else if (lcdMode == 0x03) {
                    // Do nothing
                }
            }
        }
        reset_cursor_pos();
        //write_char_to_LCD('A');
        //write_char_to_LCD('B');
        //write_char_to_LCD('C');
        _delay_ms(250);
        //double_toggle_enable();
        if (lcdMode == 0x00) {
            // get Temp
            ADCval = read_analog();
    		display_analog_decimal(ADCval);
        } else if (lcdMode == 0x01) {
            // get milage
            display_feet(feet_taveled);
        } else if (lcdMode == 0x02) {
            // get timer
            display_time(total_time);
        } else if (lcdMode == 0x03) {
            //get speed
            display_speed(current_speed);
        }

        //display_analog_binary(ADCval);
    }
    return 0;
}

ISR(INT0_vect) {
    if (bit_is_clear(CTRL_PIN, REED_PIN)) {
        _delay_us(1000);
        if (bit_is_clear(CTRL_PIN, REED_PIN)) {
            // next revolution is triggered here
            //clear_LCD();
            if (flag == 0x00) {
                feet_taveled = feet_taveled + (6); // based on wheel tire_size // FOR some reason it reads the interrupt twice
                //ticks_per_revolution = ticks_per_revolution;
                current_speed = tire_size / ticks_per_revolution;
                ticks_per_revolution = 0;
                flag = 0xff;
            } else {
                flag = 0x00;
            }
        }
    }
}

// every hundreth of a second is called
ISR(TIMER0_COMPA_vect) {    // yay theres an interrupt! http://www.nongnu.org/avr-libc/user-manual/group__avr__interrupts.html
    ticks_per_revolution = ticks_per_revolution + 1;
    total_time = total_time + 1;
}
