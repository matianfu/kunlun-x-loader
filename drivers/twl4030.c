/*
 * Copyright (C) 2007-2008 Texas Instruments, Inc.
 *
 * USB
 * Imported from omap3-dev drivers/usb/twl4030_usb.c
 * This is unique part of the copyright
 *
 * twl4030_usb - TWL4030 USB transceiver, talking to OMAP OTG controller
 *
 * (C) Copyright 2009 Atin Malaviya (atin.malaviya@gmail.com)
 *
 * Based on: twl4030_usb.c in linux 2.6 (drivers/i2c/chips/twl4030_usb.c)
 * Copyright (C) 2004-2007 Texas Instruments
 * Copyright (C) 2008 Nokia Corporation
 * Contact: Felipe Balbi <felipe.balbi@nokia.com>
 *
 * Author: Atin Malaviya (atin.malaviya@gmail.com)
 *
 *
 * Keypad
 *
 * (C) Copyright 2009
 * Windriver, <www.windriver.com>
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */
#include <twl4030.h>
#include <asm/io.h>
/*
 * Battery
 */
 /*******************************************************
 * Routine: delay
 * Description: spinning delay to use before udelay works
 ******************************************************/
static inline void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
			  "bne 1b":"=r" (loops):"0"(loops));
}
static inline void udelay(unsigned long us)
{
	delay(us * 200); /* approximate */
}
#define mdelay(n) ({ unsigned long msec = (n); while (msec--) udelay(1000); })

/* Functions to read and write from TWL4030 */
static inline int twl4030_i2c_write_u8(u8 chip_no, u8 val, u8 reg)
{
	return i2c_write(chip_no, reg, 1, &val, 1);
}

static inline int twl4030_i2c_read_u8(u8 chip_no, u8 *val, u8 reg)
{
	return i2c_read(chip_no, reg, 1, val, 1);
}

/*
 * Sets and clears bits on an given register on a given module
 */
static inline int clear_n_set(u8 chip_no, u8 clear, u8 set, u8 reg)
{
	int ret;
	u8 val = 0;

	/* Gets the initial register value */
	ret = twl4030_i2c_read_u8(chip_no, &val, reg);
	if (ret) {
		printf("a\n");
		return ret;
	}

	/* Clearing all those bits to clear */
	val &= ~(clear);

	/* Setting all those bits to set */
	val |= set;

	/* Update the register */
	ret = twl4030_i2c_write_u8(chip_no, val, reg);
	if (ret) {
		printf("b\n");
		return ret;
	}
	return 0;
}

static int twl4030_chgi_ref_set(int level)
{
	int ret;
       u8 lsb=0, msb=0;
	u8 val;

	twl4030_i2c_read_u8(TWL4030_CHIP_MAIN_CHARGE, &val, REG_BCICTL1);
	if (val & CGAIN){
            /* slope of 3.33 mA/LSB */
           level = (level * BCI_CHGI_REF_STEP) / (BCI_CHGI_REF_PSR_R1);
	}
	else{
            /* slope of 1.665 mA/LSB */
	     level = (level * BCI_CHGI_REF_STEP) / (BCI_CHGI_REF_PSR_R2);
	}

       if(level > BCI_CHGI_REF_MASK){
            level = BCI_CHGI_REF_MASK;
       }

       level |= BCI_CHGI_REF_OFF;

       lsb = (u8)(level & CHGI_LSB);
       msb = (u8)((level >> BCI_REG_SIZE) & CHGI_MSB);

       /* Change charging current */
	twl4030_i2c_write_u8(TWL4030_CHIP_MAIN_CHARGE, 0xE7,
			REG_BCIMFKEY);

       ret = twl4030_i2c_write_u8(TWL4030_CHIP_MAIN_CHARGE,
			      lsb,
			      REG_BCIIREF1);

       twl4030_i2c_write_u8(TWL4030_CHIP_MAIN_CHARGE, 0xE7,
			REG_BCIMFKEY);

       ret = twl4030_i2c_write_u8(TWL4030_CHIP_MAIN_CHARGE,
			      msb,
			      REG_BCIIREF2);

	return ret;
}

int twl4030_setup(void)
{
       int ret = 0;
       int i=0;

	i2c_init(CFG_I2C_SPEED, CFG_I2C_SLAVE);
       /* Enable AC charging : ROM code is shutting down MADC CLK */
	ret = clear_n_set(TWL4030_CHIP_INTBR, 0,
		(MADC_HFCLK_EN | DEFAULT_MADC_CLK_EN), REG_GPBR1);

	udelay(100);

	ret = clear_n_set(TWL4030_CHIP_PM_MASTER,
				BCIAUTOAC | CVENAC,
				0,
				REG_BOOT_BCI);

       /* Set CGAIN=1 to make the low CHGI is 200mA*/
	ret = clear_n_set(TWL4030_CHIP_MAIN_CHARGE,
				0,
				CGAIN,
				REG_BCICTL1);

	/*set charge current to 450mA after MADC clock is enable*/
	twl4030_chgi_ref_set(DEFAULT_CHARGE_CURRENT);

	ret = clear_n_set(TWL4030_CHIP_PM_MASTER,
				0,
				BCIAUTOAC | CVENAC,
				REG_BOOT_BCI);
       return ret;
}
