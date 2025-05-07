/* 
 * File:   main.c
 * Author: chris
 *
 * Created on 05 March 2025, 11:46
 */

#include <avr/io.h>
#include "USI_TWI_Master.h"
#include "si5351a_registers.h"

#define MESSAGEBUF_SIZE 3

// for linker, emulator, and programmer's sake
#include "avr_mcu_section.h"
AVR_MCU(F_CPU, "attiny85");

unsigned int send_5351_configuration(uint8_t config);
uint8_t read_cs_pins();

/*
 * 
 */
int main(void) {
    
    /**
     * Pseudo-code
     * - uC Init, Setup fuses(?), PortB pin directions, blah blah
     * - Select default clock speed (50.35MHz, CS=000) and send that to Skywork chip
     * - Loop forever:
     * -- Check Clock Selection pins
     * -- If the pins change and stay that way for X seconds (debouncing)
     * -- THEN: send the appropriate Clock selection to Skyworks clock gen
     * --- Run through the flow chart - disable clocks, power down, etc...
     * --- Set deltas for selected mode (store in some .h)
     * --- Reset PLLs and re-enable clocks
     * @return 
     */
    
//    configs[0] = *si5351a_init_registers;
    
	unsigned char clksel, old_clksel;

    USI_TWI_Master_Initialise();

    asm("sei");
    
    clksel = 0;
    
    // CS[2:0] = PB1,PB4,PB3
    // set Port B 3, 4 & 1 as an input?
    DDRB &= ~(1 << 1);
    DDRB &= ~(1 << 3);
    DDRB &= ~(1 << 4);
    
    // Initial Full Config
    send_5351_configuration(254);
    
    // TODO: Not sure that my INPUT pins are working as it's still seems to be
    // on clksel = 0, when it should be clksel = 1
    clksel = read_cs_pins();
    send_5351_configuration(clksel);
    
    while(1) {
        old_clksel = clksel;
        clksel = read_cs_pins();
        if(clksel != old_clksel) {
            send_5351_configuration(clksel);
        }
    }

//    result = USI_TWI_Start_Transceiver_With_Data(messageBuf, 2);
    
    
// TESTING RUBBISH 
//    // set pin 0 as output
//    DDRB |= (1 << PB0);
//    // set pin 1 as an input?
//    DDRB &= (1 << PB1);
//    // Set Pin 0 to output
//    //PORTB |= (1 << PB0);
//    
//    while(1) {
//        PINB |= (1 << PINB0);
//        _delay_us(100);
//    }
    


}

unsigned int send_5351_configuration(uint8_t config) {
    unsigned char TWI_targetSlaveAddress, result = 0;
    unsigned int i;
    unsigned char messageBuf[MESSAGEBUF_SIZE];
    
    TWI_targetSlaveAddress = 0x60;

    // Set the device address - write mode - all messages are writes.
    messageBuf[0] = (TWI_targetSlaveAddress << TWI_ADR_BITS) | (FALSE << TWI_READ_BIT);
    
    // Disable Outputs
    messageBuf[1] = 0x03;
    messageBuf[2] = 0xFF;
    result = USI_TWI_Start_Transceiver_With_Data(messageBuf, 3);
    if(!result) {
        goto finish;
    }
    
    // Power Down Output Drivers
    for(i = 16; i < 24; i++) {
        messageBuf[1] = (unsigned char) i & 0xFF;
        messageBuf[2] = 0x80;
        result = USI_TWI_Start_Transceiver_With_Data(messageBuf, 3);
        if(!result) {
            goto finish;
        }
    }
    
    // Set Interrupt Masks?
    
    // Write new config
    // if it's 254 then do the initial config...
    if(config == 254) {
        for(i = 0; i < SI5351A_REVB_FULL_REG_CONFIG_NUM_REGS; i++) {
            messageBuf[1] = si5351a_revb_init_registers[i].address;
            messageBuf[2] = si5351a_revb_init_registers[i].value;
            result = USI_TWI_Start_Transceiver_With_Data(messageBuf, 3);
            if(!result) {
                goto finish;
            }
        }
    } else {
        for(i = 0; i < SI5351A_REVB_DELTA_REG_CONFIG_NUM_REGS; i++) {
            messageBuf[1] = si5351a_revb_registers[config][i].address;
            messageBuf[2] = si5351a_revb_registers[config][i].value;
            result = USI_TWI_Start_Transceiver_With_Data(messageBuf, 3);
            if(!result) {
                goto finish;
            }
        }
    }
    
    // PLLA & B Soft Reset
    messageBuf[1] = 0xB1; //177;
    messageBuf[2] = 0xA0;
    result = USI_TWI_Start_Transceiver_With_Data(messageBuf, 3);
    if(!result) {
        goto finish;
    }
    
    // Enable Outputs
    messageBuf[1] = 0x03;
    messageBuf[2] = 0xFC;
    result = USI_TWI_Start_Transceiver_With_Data(messageBuf, 3);
    if(!result) {
        goto finish;
    }
    
    
    finish:
      asm("nop");
      return 0;
}

uint8_t read_cs_pins() {
    uint8_t clock_select = 0;
    uint8_t cs0 = 0;
    uint8_t cs1 = 0;
    uint8_t cs2 = 0;

    
    cs2 = PINB & (1 << 1);
    cs1 = PINB & (1 << 4);
    cs0 = PINB & (1 << 3);
    clock_select = (cs0 >> 3) | (cs1 >> 3) | (cs2 << 1);
//    unsigned char temp;
    
//    clock_select = (PINB & 0x02);
//    clock_select = (clock_select & 0x1A);
    
    
//    clock_select = temp;
//    clock_select = clock_select >> 1;
//    clock_select |= ( temp & (1 << 1) ) >> 1;
//    clock_select |= ( temp & (1 << 3) ) >> 3;
//    clock_select |= ( temp & (1 << 4) ) >> 4;
    if(clock_select > 7) {
        // Default to 0
        clock_select = 0;
    }
    return clock_select;
}
