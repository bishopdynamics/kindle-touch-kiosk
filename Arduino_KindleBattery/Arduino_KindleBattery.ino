
/*
 * Kindle Touch (D01200) Battery Emulator for Arduino
 *   This is NOT SMBus battery manager, but a non-standard one operating via normal I2C
 *  Mostly copied from: https://github.com/mciantyre/mock-smbus-arduino
 *  Plus extra info from:  http://bloodsweatandsolder.blogspot.com/2016/04/booting-kindle-dx-graphite-without.html
 *      code repo: https://github.com/naramsay/pic_kindle_dx_battery_spoof
 *      Node that the model in quesion (kindle_dx) did not apparently check for valid battery id, but ours (kindle touch) does.
 *      Also note that this model
 * 
  */


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


#include <Wire.h>
// using AltSoftSerial library
#include <SoftwareSerial.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "commands.h"


#define SOFTSERIAL_TX 3
#define SOFTSERIAL_RX 2
#define SOFTSERIAL_BAUD 115200
#define BATTERY_ID 72         // my device is a "whitney" so will use 72 -> 0x48
#define BATTERY_ADDRESS 0xAA  // multiple sources indicate 0xAA is common for many, if not all kindles

#define I2C_TIMEOUT 100
#define I2C_NOINTERRUPT 1

//SoftwareSerial softSerial(SOFTSERIAL_RX, SOFTSERIAL_TX);
#define softSerial Serial

#define BUFFER_MAX_LEN    (     64 )

// typedef doesn't seem to work here...?
// Not attempting to figure it out. #define instead
#define batt_t int16_t
#define addr_t uint8_t

typedef struct
{
  const char* key;
  const addr_t addr;
  batt_t val;
} smbus_lookup;


/**
 * Add keys, addresses, and default values here
 */
static smbus_lookup lookups[] = {
/*     Serial key   |   SMBUS command addr    |  Initial value  |   Units  */
/* -----------------|-------------------------|-----------------|--------- */
  { "YOSHI_BATTERY_ID"  , YOSHI_BATTERY_ID     , BATTERY_ID }    // kindle battery identifier
};
#define NUM_LOOKUPS ( sizeof( lookups ) )

/**
 * Get the value associated with key
 */
batt_t getvalkey( const char* inkey )
{
  size_t i;
  for ( i = 0; i < NUM_LOOKUPS; ++i )
  {
    if ( 0 == strcmp( lookups[i].key, inkey ) )
    {
      return lookups[i].val;
    }
  }

  return ( -1 );
}

/**
 * Get the value associated with addr
 */
batt_t getvaladdr( const addr_t addr )
{
  size_t i;
  for ( i = 0; i < NUM_LOOKUPS; ++i )
  {
    if ( addr == lookups[i].addr )
    {
      return lookups[i].val;
    }
  }

  return ( -1 );
}

/**
 * Set the value of the state identified by key
 */
int setval( const char* inkey, const batt_t val )
{
  size_t i;
  for ( i = 0; i < NUM_LOOKUPS; ++i )
  {
    if ( 0 == strcmp( lookups[i].key, inkey ) )
    {
      lookups[i].val = val;
      return 1;
    }
  }

  return 0;
}

//
// The next value to send
// when requested
//
static volatile batt_t next_send = -1;

//
// I2C callbacks
//

/**
 * Callback when bytes are received
 * Sets the next written value
 */
void receivehandler( int nbytes )
{
  char c = Wire.read();
  softSerial.print( "receivehandler read: " ); softSerial.println( c );
  batt_t val = -1;
  if ( -1 != ( val = getvaladdr( c ) ) )
  {
    next_send = val;
  }
}

/**
 * Write data back to master LSB
 */
void requesthandler( void )
{
  byte tx[2] = { ( next_send >> 8 ) & 0xFF
               , ( next_send & 0xFF ) };
  char tx_char[2];
  sprintf(tx_char, "%02X", tx);
  softSerial.print( "requesthandler writing: 0x" ); softSerial.println( tx_char );
  Wire.write( tx, 2);
}

//
// Serial methods
//
void processSerial( char* buffer, size_t len )
{
  char * key, * value, * i;
  key = strtok_r( buffer, "=", &i );
  value = strtok_r( NULL, "=", &i );

  if ( NULL == value )
  {
    softSerial.println("ERROR: key=value pair not provided");
    return;
  }

  batt_t v = atoi( value );
  if ( 0 == strcmp( value, "0" ) )
  {
    v = 0;
  }
  else if ( 0 == v )
  {
   softSerial.println("ERROR: NaN");
   return; 
  }
  
  if ( 1 == setval( key, v ) )
  {
    char okmsg[BUFFER_MAX_LEN] = {0};
    snprintf( okmsg, BUFFER_MAX_LEN, "set %s=%s", key, value );
    softSerial.println( okmsg );
  }
  else
  {
    softSerial.println("ERROR: setting key=value failed (key not valid)");
  }
}

//
// Arduino defaults
//

void setup() {
  // setup software serial for debugging first
  softSerial.begin(SOFTSERIAL_BAUD);
  softSerial.println( "SoftwareSerial is working" );

  // init I2C
  Wire.begin( BATTERY_ADDRESS );
  Wire.onReceive( receivehandler );
  Wire.onRequest( requesthandler );
  char addr_char[2];
  sprintf(addr_char, "%02X", BATTERY_ADDRESS);
  softSerial.print( "I2C initialized with slave address: 0x" ); softSerial.print( addr_char );
  softSerial.print( " aka " ); softSerial.println( BATTERY_ADDRESS );

}

void loop() {

  static char buffer[BUFFER_MAX_LEN] = { 0 };
  size_t c = 0;
  if ( softSerial.available() > 0 )
  {
    c = softSerial.readBytes( buffer, BUFFER_MAX_LEN );
    processSerial( buffer, c );
    memset( buffer, 0, sizeof( buffer ) );
    c = 0;
  }

}
