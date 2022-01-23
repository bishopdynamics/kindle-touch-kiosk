
/*
    Emulation of a smart li-ion battery for Kindle Touch model D01200.
    The board is code named yoshi, and the revision is whitney.
    This corresponds to a battery ID of 72, chosen as per below

    This is pulled from the gpl release of the last kernel released for the kindle touch.

    (battery_id.c)
    const int tequila_valid_ids[] = { 12, 76, 140, 204 };
    const int whitney_valid_ids[] = { 8, 72, 136, 200 };
    const int celeste_valid_ids[] = { 10, 74, 138, 202 };

    (yoshi_battery.c)
    also 0x42 (int 66) is the workbench test battery, bypasses battery check but only when in diags mode

    This repo is a convenient reference, though it is a fork of an older version that predates the celeste revision:
    https://github.com/fread-ink/kernel-k4-usb-otg/blob/master/drivers/power/yoshi_battery.c
    https://github.com/fread-ink/kernel-k4-usb-otg/blob/master/include/boardid.h
    https://github.com/fread-ink/kernel-k4-usb-otg/blob/master/include/battery_id.h

    */

// I2C requests values by address, with 128 possible addresses
//      Here is what those addresses mean for the Yoshi battery
//      The actual values here have been edited to present a 100% charged battery in good condition
//      The baseline is a capture from a different battery, from the Kindle DX (captured by neil), and we do not know what all the other values are
volatile unsigned char I2C_Array[128] = {
    0x00, // 0x00 - YOSHI_CTRL
    0x40, // 0x01 - YOSHI_MODE
    0xb6, // 0x02 - YOSHI_AR_L
    0x17, // 0x03 - YOSHI_AR_H
    0x26, // 0x04
    0x00, // 0x05
    0xa1, // 0x06 - YOSHI_TEMP_LOW - 23.75Celsius
    0x04, // 0x07 - YOSHI_TEMP_HIGH
    0xd4, // 0x08 - YOSHI_VOLTAGE_LOW - 3796mV
    0x0e, // 0x09 - YOSHI_VOLTAGE_HIGH
    0x04, // 0x0A - YOSHI_FLAGS
    0x64, // 0x0B - YOSHI_RSOC - Relative state of charge - 100%
    0xb7, // 0x0C - YOSHI_NAC_L - Nominal Available Capacity
    0x0f, // 0x0D - YOSHI_NAC_H
    0xd8, // 0x0E - YOSHI_LMD_L - Last Measured Discharge
    0x0f, // 0x0F - YOSHI_LMD_H
    0xb7, // 0x10 - YOSHI_CAC_L - milliamp-hour
    0x0f, // 0x11 - YOSHI_CAC_H
    0xd8, // 0x12
    0x0f, // 0x13
    0x67, // 0x14 - YOSHI_AI_LO - Average Current
    0x00, // 0x15 - YOSHI_AI_HI
    0x27, // 0x16
    0x09, // 0x17
    0xff,
    0xff,
    0x67,
    0x00,
    0x27,
    0x09,
    0x00,
    0x00, // 0x1F
    0x35,
    0x0e,
    0x0f,
    0x00,
    0x0f,
    0x00,
    0x82,
    0x08, // 0x27 -
    0x02, // 0x28 - YOSHI_CYCL_L - Cycle Count this session? (i think 10 hrs) - 2 cycles
    0x00, // 0x29 - YOSHI_CYCL_H
    0x02, // 0x2A - YOSHI_CYCT_L - Cycle Count Total? - 2 cycles
    0x00, // 0x2B - YOSHI_CYCT_H
    0x64, // 0x2C - YOSHI_CSOC - Compensated state of charge  100%
    0xe0, // 0x2D
    0xa0, // 0x2E
    0x07, // 0x2F
    0x21,
    0x00,
    0xf8,
    0x9b,
    0xc3,
    0x1b,
    0xff,
    0xff, // 0x37
    0xa8,
    0xf6,
    0xff,
    0x00,
    0xe3,
    0x03,
    0x4b,
    0x4e, // 0x3F
    0x00,
    0x00,
    0x05,
    0x00,
    0x00,
    0x00,
    0x21,
    0xa9, // 0x47
    0xc7,
    0x48,
    0x38,
    0x85,
    0x7f,
    0x66,
    0x6c,
    0x46, // 0x4F
    0xdf,
    0x20,
    0x36,
    0x09,
    0x00,
    0x01,
    0x52,
    0xa4, // 0x57
    0xcf,
    0x0f,
    0x3f,
    0x24,
    0x00,
    0x00,
    0x71,
    0xff, // 0x5F
    0x91,
    0xff,
    0x62,
    0xff,
    0x8d,
    0x04,
    0xd9,
    0x05, // 0x67
    0x00,
    0x00,
    0x00,
    0x01,
    0x00,
    0x07,
    0x00,
    0x00, // 0x6F
    0x2d,
    0x78,
    0x37,
    0x6d,
    0x7a,
    0xbb,
    0x21,
    0xa9, // 0x77
    0xc7,
    0x48,
    0x38,
    0x85,
    0x7f,
    0x66,
    0x48, // 0x7E - YOSHI_BATTERY_ID - Battery ID - yoshi_battery.c validates that battery ID belongs to one of 4 possible values, which are different across 3 board revisions
    0xa9  // 0x7F
};


