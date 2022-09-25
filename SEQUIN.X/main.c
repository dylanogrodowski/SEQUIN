// SEQUIN
// 9/25/2022
// Dylan Ogrodowski
// Version 0.13

// TO DO:
// RECORD LED AND PLAYBACK LED CONFIGURATION
// ADC0 will be designated replacement for PREV
// New rx receiver logic
// If prev data exists, clear mem

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "memory.h"
#include "uart.h"
#include "commands.h"
#include "gpio.h"

FUSES = {
	.low = 0xEF, // LOW {SUT_CKSEL=EXTXOSC_8MHZ_XX_16KCK_4MS1, CKOUT=CLEAR, CKDIV8=CLEAR}
	.high = 0x19, // HIGH {BOOTRST=CLEAR, BOOTSZ=2048W_3800, EESAVE=CLEAR, WDTON=CLEAR, SPIEN=SET, JTAGEN=SET, OCDEN=SET}
	.extended = 0xFF, // EXTENDED {RSTDISBL=CLEAR, BODLEVEL=DISABLED}
};

LOCKBITS = 0xFF; // {LB=NO_LOCK, BLB0=NO_LOCK, BLB1=NO_LOCK}

#define F_CPU 16000000UL

// FSM modes
#define IDLE 1
#define RECORD 2
#define PLAYBACK 3

#define PREV_BITMASK 0x01
#define NEXT_BITMASK 0x02
#define REC_SW_BITMASK 0x04
#define PLAYBACK_SW_BITMASK 0x08
#define CLOCK_OFF_BITMASK 0xF8

#define DEBOUNCE_DELAY 25 // ms

#define TRUE 1
#define FALSE 0

// Function prototypes
void init_timer();
void next_track();
void prev_track();
void rec_startup_sequence();
void playback_startup_sequence();
void mem_write_autoaddress(uint8_t data);
uint8_t debounce(uint8_t bitmask);

volatile uint8_t overflow_count = 0;
volatile uint8_t new_rx_received = 0;
volatile uint8_t waiting_for_compare_interrupt = 0;

uint8_t target_delay_low = 0;
uint8_t target_delay_high = 0;
uint8_t target_delay_overflow = 0;

uint8_t current_track = 0;
uint8_t addr_h = 0;
uint8_t addr_m = 0;
uint16_t local_addr = 0; // 12-bit byte position within the 4 kB track
uint8_t mode = IDLE;
uint8_t switches = 0;

uint8_t expecting_data = 0; // Flag that is set when a two-byte command is received, and cleared when the data byte is read

int main(void) 
{
    init_GPIO();
    init_SPI();
    init_timer();
    init_uart();
    
    uint8_t manufacturer_ID = mem_manufacturer_ID(); // DEBUG: Memory test
    
    /*
    // Memory test sequence
    mem_write_single(0xAA, 0x00, 0x00, 0x00); // DEBUG: Memory test
    uint8_t test_data = mem_read_single(0x00, 0x00, 0x00); // DEBUG: Memory test
    mem_erase();
    mem_wait();
    mem_write_single(0xFD, 0x00, 0x00, 0x00); // DEBUG: Memory test
    test_data = mem_read_single(0x00, 0x00, 0x00); // DEBUG: Memory test
    asm("NOP");
    */
    
    sei(); // Enable global interrupts
    
    while (1) 
    {
        switch(mode)
        {
            case(IDLE):
                // Check for button presses, update LCD
                switches = PINJ | (PINF & PREV_BITMASK);
                if (switches & PREV_BITMASK)
                {
                    if(!debounce(PREV_BITMASK)) break; // Restart loop if press was a bounce
                    prev_track();
                }
                else if (switches & NEXT_BITMASK)
                {
                    if(!debounce(NEXT_BITMASK)) break; // Restart loop if press was a bounce
                    next_track();
                }
                else if (switches & REC_SW_BITMASK)
                {
                    if(!debounce(REC_SW_BITMASK)) break;
                    rec_startup_sequence();
                }
                else if (switches & PLAYBACK_SW_BITMASK)
                {
                    if(!debounce(PLAYBACK_SW_BITMASK)) break;
                    playback_startup_sequence();
                }
                break;
            case(RECORD):
                while (!(UCSR0A & UART_RX_COMPLETE_BITMASK)) // Wait for the next uart packet
                {
                    // Poll the REC button. If pressed, ends the recording:
                    if (switches & REC_SW_BITMASK)
                    {
                        if(!debounce(REC_SW_BITMASK)) break;

                        // Clear the timer
                        overflow_count = 0;
                        TCNT1H = 0x00;
                        TCNT1L = 0x00;
                        // Update LCD
                        
                        uart_send(SYN_REC_END); // Inform the synthesizer of end-of-recording so it stops sending data
                        mode = IDLE;
                    }
                }
                    
                // Once a UART packet is received...
                uint8_t data = UDR0; // Read the data
                if (!expecting_data) // Interpret as a command
                {
                    TCCR1B &= CLOCK_OFF_BITMASK; // Stop the timer
                    uint8_t duration_l = TCNT1L; // Low byte must be read before the high byte
                    uint8_t duration_m = TCNT1H;
                    uint8_t duration_h = overflow_count;

                    // Clear the timer
                    overflow_count = 0;
                    TCNT1H = 0x00;
                    TCNT1L = 0x00;

                    // Start the timer
                    TCCR1B |= 0x02; // Prescaler of 1/8 from I/O clock

                    // Send the duration followed by the command, MSB first
                    mem_write_autoaddress(duration_h);
                    mem_write_autoaddress(duration_m);
                    mem_write_autoaddress(duration_l);
                    mem_write_autoaddress(data);
                    
                    // Check if the command is followed by a data byte and set the flag accordingly
                    if (data > 0x84) // Threshold for two-byte commands
                    {
                        expecting_data = 1;
                    }
                    else
                    {
                        expecting_data = 0;
                    }
                }
                else // Interpret as data, send straight to memory
                {
                    // Send the data byte
                    mem_write_autoaddress(data);
                    expecting_data = 0;
                }

                break;
            case(PLAYBACK):
                if (overflow_count == target_delay_overflow)
                {
                    // FINISH THIS
                    // DO ONCE:
                    
                    waiting_for_compare_interrupt = 1; // Cleared in interrupt
                    TIMSK1 |= 0x02; // Enable the compare match interrupt
                    while(waiting_for_compare_interrupt)
                    {
                        asm("NOP"); 
                    }
                    
                    uint8_t command = spi_read(); // Read command from external memory
                    uart_send(command); // Send the command to the synth
                    if (command > 0x84) // Threshold for two-byte commands
                    {
                        uint8_t data = spi_read(); // Read the attached data (if applicable) from external memory
                        uart_send(data);
                    }

                    // Read timing from external memory for next cycle
                    target_delay_overflow = spi_read(); 
                    target_delay_high = spi_read(); // Read high byte for CMPA of TIMER1 from memory
                    target_delay_low = spi_read();  // Read the next timing from external memory
                    
                    overflow_count = 0; // Set overflow count to zero 
                    OCR1AH = target_delay_high; // Set the compare match target
                    OCR1AL = target_delay_low; // Set the compare match target
                }
                else
                {
                    // Poll inputs
                }
                
                
                break;
        }
        asm("NOP"); // DEBUG: Breakpoint location
    }
}

void init_timer()
{
    // Set top to 0xFFFF
    ICR1H = 0xFF; 
    ICR1L = 0xFF;
    
    TCCR1A &= 0x0C; // Normal mode, overflow at 0xFFFF
    //TCCR1B |= 0x02; // Prescaler of 1/8 from I/O clock
    TIMSK1 |= 0x01; // Enable TCC1 overflow interrupt
}

void next_track()
{
    current_track++; // Change the current track
    addr_h = (current_track & 0xF0) >> 4;
    addr_m = (current_track & 0x0F) << 4;
    local_addr = 0; // 12-bit byte position within the 4 kB track
    // Update the LCD
}

void prev_track()
{
    current_track--; // Change the current track
    addr_h = (current_track & 0xF0) >> 4;
    addr_m = (current_track & 0x0F) << 4;
    local_addr = 0; // 12-bit byte position within the 4 kB track
    // Update the LCD
}

void rec_startup_sequence()
{
    // Initiate record mode
    // Clear the timer
    TCCR1B &= CLOCK_OFF_BITMASK; // Stop the timer (should already be off)
    TCNT1H = 0x00;
    TCNT1L = 0x00;
    overflow_count = 0x00; // Clear the overflow counter
    
    mem_write_single(SYN_REC_START, addr_h, addr_m, 0x00); // Place a start command in memory
    local_addr = 1; // Reset the local_addr, starting at one to account for the start command at location zero
    uart_send(SYN_REC_START); // Send start recording command over UART

    // Wait for all initialization data to be received
    uint8_t startup_done = 0;
    while(!startup_done)
    {
        if (UCSR0A & UART_RX_COMPLETE_BITMASK)
        {
            uint8_t data = UDR0; // Read the data register

            // Initial states are stored without duration data
            // The SYN_STARTUP_DONE command is also stored to memory to signify a switch to duration-based commands during playback
            mem_write_autoaddress(data);

            if (data == SYN_STARTUP_DONE)
            {
                while (!(UCSR0A & UART_RX_COMPLETE_BITMASK)); // Wait for the next uart packet
                data = UDR0;

                // Write the followup data to memory, regardless of if it is the second done flag or not
                mem_write_autoaddress(data);
                
                if (data == SYN_STARTUP_DONE) // If this is a true startup complete market, no commands need to be sent to the synth
                {
                    startup_done = TRUE;
                }
            }
        }
    }

    mode = RECORD; // Update the FSM
    TCCR1B |= 0x02; // Start the timer with a prescaler of 1/8 from I/O clock
}

void playback_startup_sequence()
{
    // Clear the timer
    TCCR1B &= CLOCK_OFF_BITMASK; // Stop the timer (should already be off)
    TCNT1H = 0x00;
    TCNT1L = 0x00;
    overflow_count = 0x00; // Clear the overflow counter
    
    // TODO: IMPLEMENT
    uint8_t data = mem_read_single(addr_h, addr_m, 0x00); // Read 1st location in track mem
    if (data != SYN_REC_START) // Check if the track is blank or corrupted
    {
        // If so, update LCD
    }
    else
    {
        uart_send(SYN_PLAYBACK_START); // Send the command to the synth to enter playback mode
        overflow_count = 0x00; // Clear the overflow counter
        local_addr = 1; // Initialize the local address to just after the SYN_REC_START data
        // Send the initialization data
        uint8_t startup_done = 0;
        while(!startup_done)
        {
            uint8_t local_addr_h = ((local_addr & 0x0F00) >> 8);
            uint8_t temp_addr_m = addr_m | local_addr_h;
            uint8_t data = mem_read_single(addr_h, temp_addr_m, (uint8_t)local_addr);
            local_addr++;

            // Startup done is signified by two sequential SYN_STARTUP_DONE bytes
            // This is done to prevent an analog value from falsely appearing as a flag to end the initialization
            if (data == SYN_STARTUP_DONE)
            {
                local_addr_h = ((local_addr & 0x0F00) >> 8);
                temp_addr_m = addr_m | local_addr_h;
                data = mem_read_single(addr_h, temp_addr_m, (uint8_t)local_addr);
                local_addr++;
                if (data == SYN_STARTUP_DONE) // If this is a true startup complete market, no commands need to be sent to the synth
                {
                    startup_done = TRUE;
                }
                else
                {
                    uart_send(SYN_STARTUP_DONE); // If false alarm, send the data as a regular packet
                    uart_send(data); // And also send the newly read packet
                }
            }
            else
            {
                uart_send(data); // Send the initialization packet
            }
        }
        mode = PLAYBACK;
        
        // Read the first delay
        uint8_t local_addr_h = ((local_addr & 0x0F00) >> 8);
        uint8_t temp_addr_m = addr_m | local_addr_h;
        
        target_delay_overflow = mem_read_start(addr_h, temp_addr_m, (uint8_t)local_addr);
        target_delay_high = spi_read(); // Read high byte for CMPA of TIMER1 from memory
        target_delay_low = spi_read(); // Read low byte for CMPA of TIMER1 from memory
        
        OCR1AH = target_delay_high; // Preconfigure CMPA for first delay cycle
        OCR1AL = target_delay_low; // Preconfigure CMPA for first delay cycle
        
        TCCR1B |= 0x02; // Start the timer with a prescaler of 1/8 from I/O clock
    }
}

void mem_write_autoaddress(uint8_t data)
{
    uint8_t local_addr_h = ((local_addr & 0x0F00) >> 8);
    uint8_t temp_addr_m = addr_m | local_addr_h;
    mem_write_single(data, addr_h, temp_addr_m, (uint8_t)local_addr); // Send the data to memory
    local_addr++;
}

// Returns FALSE (0) if the press was a bounce, TRUE (1) otherwise 
uint8_t debounce(uint8_t bitmask)
{
    // Debounce
    _delay_ms(DEBOUNCE_DELAY);
    uint8_t switches = PINJ | (PINF & PREV_BITMASK);
    if (!(switches & bitmask)) return FALSE;

    // Wait for the button to be released
    while(switches & bitmask)
    {
        switches = PINJ | (PINF & PREV_BITMASK);
    }
    return TRUE;
}

ISR(TIMER1_OVF_vect)
{
    overflow_count++;
}

ISR(TIMER1_COMPA_vect)
{
    waiting_for_compare_interrupt = 0; // Clear wait flag
    TIMSK1 &= 0xFD; // Disable the TIMER1 compare match interrupt
}