#include "uart.h"

void init_uart()
{
    DDRE |= 0x02; // Set TX pin as an output
    UCSR0B |= 0x18; // Enable RX and TX
    UCSR0C |= 0x76; // Async, odd parity, 1 stop bit
    UBRR0H |= 0x00; // Set baud rate to 9600 bps
    UBRR0L |= 0x67; // Set baud rate to 9600 bps
}

void uart_send(uint8_t data)
{
    UDR0 = data;
    while(!(UCSR0A & UART_TX_COMPLETE_BITMASK)); // Wait for transmit to complete
    UCSR0A |= UART_TX_COMPLETE_BITMASK; // Flag is cleared by writing a one
}