 /**

 *********************************************************************/

#define THIS_IS_RTC
#include "projdefs.h"
#include "rtc.h"
#include "net\xeeprom.h" 		// for re-init of i2c 
#include "net\i2c.h"
#include "net\tick.h"

#define RTC_BAUD 50000			// the I2c Clock rate for thr RTC chip , slowed down because of 2.2K in  series with the clock line

typedef enum _RTC_REGS {RTCSec,RTCMin,RTCHour,RTCDay,RTCDate,RTCMonth,RTCYear,RTCControl}
RTC_REGS;

static BYTE rtc_regs[RTCControl+1] = {0x00,0x00,0x11,0x05,0x27,0x08,0x09, 0x80};


typedef struct _tag_RTC
{
	WORD Year;
	BYTE Month;
	BYTE Date;
	BYTE Hour;
	BYTE Min;
	BYTE Sec;
	
}tRTC;

// Global in and out interface 
tRTC RTC;
	
// utility to convert to BCD var for DS1307	
static 
BYTE hextoBCD( BYTE hex)
{
	BYTE ones,tens;
	
	ones = hex % 10;
	tens = hex / 10;
	return  (ones | (tens<<4));
}	

// utility to convert from BCD var of DS1307	
static 
BYTE BCDtohex( BYTE bcd )
{
	BYTE ret;

	ret = bcd&0xf;
	ret += (bcd>>4)*10;	

	return ret;
}	

// communicates via I2c and sets the RTC chip to the date & time
// returns 1 if successful 
BYTE SetRTC( void ) // Using global RTC structure to set RTC from 
{
	BYTE i;
	BYTE ret = 1;
	
	rtc_regs[RTCYear] = hextoBCD(RTC.Year - 2000); 
	rtc_regs[RTCMonth] = hextoBCD(RTC.Month)& 0x1F;
	rtc_regs[RTCDate] = hextoBCD(RTC.Date)  & 0x3F;
	rtc_regs[RTCHour] = hextoBCD(RTC.Hour) 	& 0x3f;
	rtc_regs[RTCMin] = hextoBCD(RTC.Min) 	& 0x7f;
	rtc_regs[RTCSec] = hextoBCD(RTC.Sec)	& 0x7f;  // Bit 7 contains the "Clock Halt" bit and must be 0 for the RTC to tun 
	rtc_regs[RTCDay] = 0; 	// no support for day of the week
	rtc_regs[RTCControl] = 0x80; // No square wave output
	
	
	// init to < 100K Baud
	i2cOpen(I2C_MASTER, I2C_SLEW_OFF, EE_BAUD(CLOCK_FREQ, RTC_BAUD));
	TRISG_RG0 =1;			// enable the SCK onto the D1307
	
	i2cWaitForIdle();
    i2cPutStartAndWait();       //Write I2C Start Condition, and wait for it to complete
    i2cPutByteAndWait(D1307_ADDR | I2C_WRITE);
   
    if(i2cWasAckReceived()) 
	{
		i2cPutByteAndWait(0);		// Set reg addr to 0
		for (i = 0; i<8;i++)
			i2cPutByteAndWait(rtc_regs[i]);	// set RTC registers ignoring ack
    }
    else
    	ret = 0;
    	
 	i2cPutStopAndWait();
	LATG0 = 1;		// disables the DS1307 clock
	TRISG_RG0 =0;   // drive Ds1307 clock line high 

	
	// reset I2C back to 400K Baud
	i2cOpen(I2C_MASTER, I2C_SLEW_ON, EE_BAUD(CLOCK_FREQ, 400000));
	
	return ret;

}	


static 
void ReadRTC( void )	// Using Global struct RTC to store time into
{
	BYTE i;
	
	// init to 100k Baud
	i2cOpen(I2C_MASTER, I2C_SLEW_OFF, EE_BAUD(CLOCK_FREQ, RTC_BAUD));
	TRISG_RG0 =1;		// enables the RTC, CLK via 2.2K onto DS1307

	i2cWaitForIdle();
    i2cPutStartAndWait();       //Write I2C Start Condition, and wait for it to complete
    i2cPutByteAndWait(D1307_ADDR | I2C_WRITE);
   

    if(i2cWasAckReceived()) 		// check only once for ack 
	{
		i2cPutByteAndWait(0);		// Set address register  to 0
	
	     //Send repeated I2C start condition
        i2cRestartAndWait();

        //Send module address, and read/write bit (LSB bit) set - this is a read message
        i2cPutByteAndWait(D1307_ADDR |I2C_READ);
        if(i2cWasAckReceived()) 		// check only once for ack 
		{
	        for ( i = 0 ; i < RTCYear; i++)
	        {
		           rtc_regs[i]=i2cGetByte(I2C_ACK);  //ACK each byte except for last one. 
		    } 
			rtc_regs[i]=i2cGetByte(I2C_NACK); 		// NACK on last Byte, the Year	
		}
   	}
	
	i2cPutStopAndWait();
	
	LATG0 = 1;		// disables the DS1307 clock
	TRISG_RG0 =0;   // drive Ds1307 clock line high 
	
	
	//reset I2C back to 400K Baud
	i2cOpen(I2C_MASTER, I2C_SLEW_ON, EE_BAUD(CLOCK_FREQ, 400000));
	
	
	return ;
}	

// Access rountine to convert and retrieve the BCD values from the RTC
// Read of the chip is done in the service routine, I2C can not be read while in the 
// middle of http tag code
void GetRTC( void )
{
	RTC.Year = BCDtohex( rtc_regs[RTCYear]) + 2000;
	RTC.Month = BCDtohex( rtc_regs[RTCMonth]);
	RTC.Date = BCDtohex( rtc_regs[RTCDate]);
	RTC.Hour = BCDtohex( rtc_regs[RTCHour]);
	RTC.Min = BCDtohex( rtc_regs[RTCMin]);
	RTC.Sec = BCDtohex( rtc_regs[RTCSec]);
}	


// Servie routine to read the BCD Time/date values from the RTC
// Fills in the RTC reg array -- Need to read the RTC while not in the middle of a eeprom read, since it
// it has to shwitch the I2C clock around and that somehow conflicts with the EEPROm reading of webpages
//


void RTC_Service( void )
{
	static TICK8 trtc = 0;
	// execute once per second
	 if ( TickGetDiff8bit(trtc) >= (TICK8)TICKS_PER_SECOND  )
    {	
	    trtc = TickGet8bit();
		ReadRTC();
		
	}
	
}		
