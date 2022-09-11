#include "spi.h"

// Configure the SPI bus for interface with the W25X40CLSNIG external memory unit
void init_SPI()
{
    DDRB |= 0x07; // Configure MOSI, SCK, and \SS as outputs
    DDRB &= ~(0x08); // Configure MISO as an input
    
    // The combination of Fosc/4 and double speed results in an overall speed of Fosc/2
    SPCR |= 0x50; // SPI enable, MSB first, master mode, mode 0, Fosc/4
    SPSR |= 0x01; // Enable double SPI speed
}

uint8_t spi_read()
{
    SPDR = DUMMY_BYTE;
    while(!(SPSR & (1<<SPIF))); // Wait for the transfer to complete
    
    uint8_t read_data = SPDR;
    return read_data;
}

void spi_write(uint8_t data)
{
    SPDR = data;
    while(!(SPSR & (1<<SPIF))); // Wait for the transfer to complete
}