#include "uart.h"

void init_uart()
{
    PRR &= 0xFD; // Ensure USART0 isn't disabled by power reduction register
    DDRE |= 0x02; // Set TX pin as an output
    UBRR0H |= 0x00; // Set baud rate to 9600 bps
    UBRR0L |= 0x67; // Set baud rate to 9600 bps
    UCSR0B |= 0x18; // Enable RX and TX
    UCSR0C |= 0x36; // Async, odd parity, 1 stop bit
}

void uart_send(uint8_t data)
{
    while(!(UCSR0A & UART_DATA_EMPTY_BITMASK)); // Wait for the previous transmission to complete, if any
    UDR0 = data;
}