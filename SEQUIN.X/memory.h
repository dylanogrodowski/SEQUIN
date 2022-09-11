#include <avr/io.h>
#include "spi.h"

#define MEM_WRITE_ENABLE_ISTR 0x06
#define MEM_WRITE_ISTR 0x02
#define MEM_READ_ISTR 0x03
#define MEM_CHIP_ERASE_ISTR 0xC7 // Alternate instruction code is 0x60
#define MEM_STATUS_ISTR 0x05
#define MEM_MANUFACTURER_ID_ISTR 0x90

#define MEM_BUSY_BITMASK 0x01

void mem_write_enable();
void mem_write_single(uint8_t data, uint8_t addr_h, uint8_t addr_m, uint8_t addr_l);
uint8_t mem_read_single(uint8_t addr_h, uint8_t addr_m, uint8_t addr_l);
uint8_t mem_read_status();
void mem_wait();
uint8_t mem_manufacturer_ID();
