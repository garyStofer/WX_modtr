/**
 * @brief           
 * @file            
 * @author           
 * @dependencies    -
 * @compiler        MPLAB C18 v2.10 or higher <br>
 *                  HITECH PICC-18 V8.35PL3 or higher
 
  
 *********************************************************************/


#ifndef _RTC_H_
#define _RTC_H_

/////////////////////////////////////////////////
//Global defines

#define I2C_READ 0x01	// 		Read address bit
#define I2C_WRITE 0x00  //   

#define I2C_ACK  1
#define I2C_NACK 0

// DS1307 I2c address 
#define D1307_ADDR 0xd0 
#define D1307_ADDR_Rd 0xd1 

// TMP100 I2C address
#define TMP100_ADDR 0x94


typedef struct _tag_RTC
{
	WORD Year;
	BYTE Month;
	BYTE Date;
	BYTE Hour;
	BYTE Min;
	BYTE Sec;
	
}tRTC;


/////////////////////////////////////////////////
//Global data
extern tRTC RTC;

/////////////////////////////////////////////////
//Function prototypes
extern BYTE SetRTC( void );
extern void GetRTC( void );
extern void RTC_Service( void );

#endif   




