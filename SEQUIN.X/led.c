/*
 * File:   led.c
 * Author: Ogrod
 *
 * Created on September 25, 2022, 3:15 PM
 */


#include "led.h"

uint8_t led_states[6];
//const uint8_t bank_clks = {SET1_CLK, SET2_CLK, SET3_CLK, SET4_CLK, SET5_CLK, SET6_CLK}; 

void led_on(uint8_t key)
{
    uint8_t bank_index = key >> 3;
    
    // Get bit mask of key
    uint8_t local_index = key - bank_index;
    uint8_t bitmask = 1<<local_index;
    led_states[bank_index] |= bitmask;
    
    PORTA = led_states[bank_index]; // Push to databus databus
    PORTD |= 1 << (bank_index + 2); // Clock high, shift by two because clocks are on D2-D7
    PORTD &= CLEAR_CLK; // Clock low
}

void led_off(uint8_t key)
{
    uint8_t bank_index = key >> 3;
    
    // Get bit mask of key
    uint8_t local_index = key - bank_index;
    uint8_t bitmask = 1<<local_index;
    led_states[bank_index] &= ~bitmask;
    
    PORTA = led_states[bank_index]; // Push to databus databus
    PORTD |= 1 << (bank_index + 2); // Clock high, shift by two because clocks are on D2-D7
    PORTD &= CLEAR_CLK; // Clock low
}