#include "gpio.h"

void init_GPIO()
{
    // Configure LEDs
    PORTA &= 0x00; // Disable internal pullups on the databus
    PORTD &= 0xFC; // Disable internal pullups on all clock pins
    
    DDRA |= 0xFF; // Configure A0-A7 as an output, corresponding to DBUS1-DBUS8
    DDRD |= 0xFC; // Configure D2-D7 as outputs, corresponding to SET1_CLK - SET6_CLK
    
    PORTA &= 0x00; // Set the entire databus low
    PORTD |= (~CLEAR_CLK); // Clock all clocks high
    PORTD &= CLEAR_CLK; // Clock all clocks low
    
    // Configure NEXT, RECORD, and PLAYBACK buttons
    // PREV is rerouted in hardware to ADC0
    DDRJ &= ~(0x0E); // Configure NEXT, RECORD, PLAYBACK as inputs
    DDRF &= ~(0x01); // Configure PREV (ADC0) as an input
}