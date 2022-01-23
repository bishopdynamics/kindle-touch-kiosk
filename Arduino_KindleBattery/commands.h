#ifndef COMMANDS_H
#define COMMANDS_H

// Driver Reference: https://github.com/fread-ink/kernel-k4-usb-otg/blob/master/drivers/power/yoshi_battery.c

#define YOSHI_CTRL       			0x00
#define YOSHI_MODE       			0x01
#define YOSHI_AR_L       			0x02
#define YOSHI_AR_H       			0x03


#define YOSHI_TEMP_LOW   			0x06
#define YOSHI_TEMP_HI    			0x07
#define YOSHI_VOLTAGE_LOW			0x08
#define YOSHI_VOLTAGE_HI 			0x09
#define YOSHI_FLAGS      			0x0a
#define YOSHI_RSOC  				0x0B    /* Relative state of charge */
#define YOSHI_NAC_L      			0x0C
#define YOSHI_NAC_H      			0x0D
#define YOSHI_LMD_L      			0x0E
#define YOSHI_LMD_H      			0x0F
#define YOSHI_CAC_L      			0x10    /* milliamp-hour */
#define YOSHI_CAC_H      			0x11

#define YOSHI_AI_LO      			0x14    /* Average Current */
#define YOSHI_AI_HI      			0x15


#define YOSHI_CYCL_L     			0x28
#define YOSHI_CYCL_H     			0x29
#define YOSHI_CYCT_L     			0x2A
#define YOSHI_CYCT_H     			0x2B
#define YOSHI_CSOC       			0x2c    /* Compensated state of charge */

#define YOSHI_BATTERY_ID 			0x7E    /* Battery ID - yoshi_battery.c validates that battery ID belongs to one of 4 possible values, which are different across 3 board revisions */

#endif // COMMANDS_H
