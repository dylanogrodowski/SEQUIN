#include <avr/io.h>

#define SET1_CLK 0x04
#define SET2_CLK 0x08
#define SET3_CLK 0x10
#define SET4_CLK 0x20
#define SET5_CLK 0x40
#define SET6_CLK 0x80
#define CLEAR_CLK 0x03

void init_GPIO();