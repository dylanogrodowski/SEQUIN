#include "memory.h"

void mem_write_enable()
{
    PORTB &= ~CS_BITMASK; // Set CS low
    spi_write(MEM_WRITE_ENABLE_ISTR);
    PORTB |= CS_BITMASK; // Set CS high
}

void mem_write_single(uint8_t data, uint8_t addr_h, uint8_t addr_m, uint8_t addr_l)
{
    mem_write_enable();
    
    PORTB &= ~CS_BITMASK; // Set CS low
    spi_write(MEM_WRITE_ISTR);
    spi_write(addr_h);
    spi_write(addr_m);
    spi_write(addr_l);
    spi_write(data);
    PORTB |= CS_BITMASK; // Set CS high
}

uint8_t mem_read_single(uint8_t addr_h, uint8_t addr_m, uint8_t addr_l)
{
    PORTB &= ~CS_BITMASK; // Set CS low
    spi_write(MEM_READ_ISTR);
    spi_write(addr_h);
    spi_write(addr_m);
    spi_write(addr_l);
    uint8_t data = spi_read();
    PORTB |= CS_BITMASK; // Set CS high
    
    return data;
}

// Start a read without closing it to allow for sequential reads
// Returns the data value at the start location
uint8_t mem_read_start(uint8_t addr_h, uint8_t addr_m, uint8_t addr_l)
{
    PORTB &= ~CS_BITMASK; // Set CS low
    spi_write(MEM_READ_ISTR);
    spi_write(addr_h);
    spi_write(addr_m);
    spi_write(addr_l);
    uint8_t data = spi_read();
    
    return data;
}

void mem_erase()
{
    mem_write_enable();
    
    PORTB &= ~CS_BITMASK; // Set CS low
    spi_write(MEM_CHIP_ERASE_ISTR);
    PORTB |= CS_BITMASK; // Set CS high
}

uint8_t mem_read_status()
{
    PORTB &= ~CS_BITMASK; // Set CS low
    spi_write(MEM_STATUS_ISTR);
    uint8_t data = spi_read();
    PORTB |= CS_BITMASK; // Set CS high
    
    return data;
}

// Wait until the current page program or erase operation is complete
void mem_wait()
{
    PORTB &= ~CS_BITMASK; // Set CS low
    spi_write(MEM_STATUS_ISTR);
    while(spi_read() & MEM_BUSY_BITMASK);
    PORTB |= CS_BITMASK; // Set CS high
}

// Read the manufacturer ID
// For Winbond devices, this should return 0xEF
uint8_t mem_manufacturer_ID()
{
    PORTB &= ~CS_BITMASK; // Set CS low
    spi_write(MEM_MANUFACTURER_ID_ISTR);
    spi_write(DUMMY_BYTE);
    spi_write(DUMMY_BYTE);
    spi_write(0x00); // Required to be 0x00 rather than a dummy byte
    uint8_t data = spi_read();
    PORTB |= CS_BITMASK; // Set CS high
    
    return data;
}