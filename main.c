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
    LCD_PORT = 0b00000001;              // Clear Display
    double_toggle_enable();
    // Enable Cursor TESTING ONLY
    LCD_PORT = 0b00001111;              // Cursor On
    double_toggle_enable();
}

void double_toggle_enable(void) {
    CTRL_PORT |= (1 << EN);
    _delay_ms(500);
    CTRL_PORT &= ~(1 << EN);
    _delay_ms(500);
    CTRL_PORT |= (1 << EN);
    _delay_ms(500);
}

int main(void){
    //_delay_ms(5000);
    init_AVR_pins();
    init_LCD();
    while (1) {
        double_toggle_enable();
    }
    return 0;
}
