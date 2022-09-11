#include <avr/io.h>

#define CS_BITMASK 0x01 // CS on B4
#define DUMMY_BYTE 0xBD

void init_SPI();
void spi_write(uint8_t data);
uint8_t spi_read();