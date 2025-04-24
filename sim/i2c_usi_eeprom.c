/*
	i2c_eeprom.c

	Copyright 2008, 2009 Michel Pollet <buserror@gmail.com>

 	This file is part of simavr.

	simavr is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	simavr is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with simavr.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim_avr.h"
// Todo: Get rid of TWI code...
#include "avr_twi.h"
#include "avr_usi.h"
#include "i2c_usi_eeprom.h"
#include "avr_ioport.h"


#if 1
#define DBG(x) x
#else
#define DBG(x)
#endif



static const char * _ee_irq_names[EEPROM_USI_IRQ_COUNT] = {
		[EEPROM_USI_IRQ_OUT] = "32<eeprom_usi.in",
		[EEPROM_USI_IRQ_IN] = "8>eeprom_usi.out",
		[EEPROM_USI_IRQ_SCLK] = "=eeptom_usi.clk"
};	

/*
 * called when a bit is clocked out of the USI Data Register
 */
static void
i2c_usi_eeprom_in_hook(
		struct avr_irq_t * irq,
		uint32_t value,
		void * param)
{
	i2c_usi_eeprom_t * p = (i2c_usi_eeprom_t*)param;
	avr_twi_msg_irq_t v;
	v.u.v = value;

	p->sda = value & 0x1 ? 1 : 0;
	DBG(printf("EEP ------------------- USI_IRQ_DO IRQ: p->sda = %d \n", p->sda));


	/*
	 * If we receive a STOP, check it was meant to us, and reset the transaction
	 */
	if (v.u.twi.msg & TWI_COND_STOP) {
		if (p->selected) {
			// it was us !
			if (p->verbose)
				printf("eeprom received stop\n");
		}
		p->selected = 0;
		p->index = 0;
		p->reg_addr = 0;
	}
	/*
	 * if we receive a start, reset status, check if the slave address is
	 * meant to be us, and if so reply with an ACK bit
	 */
	if (v.u.twi.msg & TWI_COND_START) {
		p->selected = 0;
		p->index = 0;
		if ((p->addr_base & ~p->addr_mask) == (v.u.twi.addr & ~p->addr_mask)) {
			// it's us !
			if (p->verbose)
				printf("eeprom received start\n");
			p->selected = v.u.twi.addr;
			avr_raise_irq(p->irq + EEPROM_USI_IRQ_OUT,  // Call the IRQ: USI_IRQ_DI if possible...
					avr_twi_irq_msg(TWI_COND_ACK, p->selected, 1));
		}
	}
	/*
	 * If it's a data transaction, first check it is meant to be us (we
	 * received the correct address and are selected)
	 */
	if (p->selected) {
		/*
		 * This is a write transaction, first receive as many address bytes
		 * as we need, then set the address register, then start
		 * writing data,
		 */
		if (v.u.twi.msg & TWI_COND_WRITE) {
			// address size is how many bytes we use for address register
			avr_raise_irq(p->irq + TWI_IRQ_INPUT,
					avr_twi_irq_msg(TWI_COND_ACK, p->selected, 1));
			int addr_size = p->size > 256 ? 2 : 1;
			if (p->index < addr_size) {
				p->reg_addr |= (v.u.twi.data << (p->index * 8));
				if (p->index == addr_size-1) {
					// add the slave address, if relevant
					p->reg_addr += ((p->selected & 1) - p->addr_base) << 7;
					if (p->verbose)
						printf("eeprom set address to 0x%04x\n", p->reg_addr);
				}
			} else {
				if (p->verbose)
					printf("eeprom WRITE data 0x%04x: %02x\n", p->reg_addr, v.u.twi.data);
				p->ee[p->reg_addr++] = v.u.twi.data;
			}
			p->reg_addr &= (p->size -1);
			p->index++;
		}
		/*
		 * It's a read transaction, just send the next byte back to the master
		 */
		if (v.u.twi.msg & TWI_COND_READ) {
			if (p->verbose)
				printf("eeprom READ data 0x%04x: %02x\n", p->reg_addr, p->ee[p->reg_addr]);
			uint8_t data = p->ee[p->reg_addr++];
			avr_raise_irq(p->irq + EEPROM_USI_IRQ_OUT,
					avr_twi_irq_msg(TWI_COND_READ, p->selected, data));
			p->reg_addr &= (p->size -1);
			p->index++;
		}
	}
}


// USI_IRQ_USCK
static void
i2c_usi_eeprom_clk_hook(
	struct avr_irq_t * irq,
	uint32_t value,
	void * param)
{
	i2c_usi_eeprom_t * p = (i2c_usi_eeprom_t*) param;
	uint8_t old_clock = p->clock_high;
	//if(value & AVR_IOPORT_OUTPUT) {
		p->clock_high = value & 0x1 ? 1 : 0;
	//}

	DBG(printf("EEP ------------------- USI_IRQ_USCK IRQ: p->clock_high = %d \n", p->clock_high));
	if(p->clock_high != old_clock) {
		if(p->clock_high == 1) {
			DBG(printf("EEP -------------------     Rising Edge \n"));
		} else {
			DBG(printf("EEP -------------------     Falling Edge \n"));
		}
	}


	// printf("I2C Clocked, value: %04x \n", value);
	// printf("EEPROM SCLK Pin State: %01x \n", p->clock_high);
}

void
i2c_usi_eeprom_init(
		struct avr_t * avr,
		i2c_usi_eeprom_t * p,
		uint8_t addr,
		uint8_t mask,
		uint8_t * data,
		size_t size)
{
	memset(p, 0, sizeof(*p));
	memset(p->ee, 0xff, sizeof(p->ee));

	p->avr = avr;
	p->addr_base = addr;
	p->addr_mask = mask;

	// TODO: Set the Clock value and SDA value to whatever the mcu pins are set to...
	p->clock_high = 1; // Pulling high so start high?
	p->sda = 1;	

	p->irq = avr_alloc_irq(&avr->irq_pool, 0, EEPROM_USI_IRQ_COUNT, _ee_irq_names);
	avr_irq_register_notify(p->irq + EEPROM_USI_IRQ_IN, i2c_usi_eeprom_in_hook, p);
	avr_irq_register_notify(p->irq + EEPROM_USI_IRQ_SCLK, i2c_usi_eeprom_clk_hook, p);

	p->size = size > sizeof(p->ee) ? sizeof(p->ee) : size;
	if (data)
		memcpy(p->ee, data, p->size);
}

void
i2c_usi_eeprom_attach(
		struct avr_t * avr,
		i2c_usi_eeprom_t * p,
		uint32_t i2c_irq_base )
{
	// connect the IRQs of the eeprom to the TWI/i2c master of the AVR
	avr_connect_irq(
		// Connect the DI of the USI module to the Data out of the EEPROM
		p->irq + EEPROM_USI_IRQ_OUT,
		avr_io_getirq(avr, i2c_irq_base, USI_IRQ_DI)); // ATTiny85 doesn't have a dedicated TWI module, but the USI which can run as TWI
		// Connect the DO of the USI module to the Data In of the EEPROM
	avr_connect_irq(
		avr_io_getirq(avr, i2c_irq_base, USI_IRQ_DO),
		p->irq + EEPROM_USI_IRQ_IN );
		// Connect the CLK IRQ with the Eeprom Clk int
	avr_connect_irq(
		avr_io_getirq(avr, i2c_irq_base, USI_IRQ_USCK),
		p->irq + EEPROM_USI_IRQ_SCLK );

}
