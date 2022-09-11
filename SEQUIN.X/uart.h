#include <avr/io.h>

#define UART_DATA_EMPTY_BITMASK  0x20
#define UART_TX_COMPLETE_BITMASK 0x40
#define UART_RX_COMPLETE_BITMASK 0x80

void init_uart();
void uart_send(uint8_t data);
