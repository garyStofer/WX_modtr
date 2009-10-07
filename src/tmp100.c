 /**

 *********************************************************************/

#define THIS_IS_TMP100
#include "projdefs.h"
#include "rtc.h"				// for I2c reated defines
#include "net\xeeprom.h" 		// for re-init of i2c 
#include "net\i2c.h"
#include "net\tick.h"

#ifdef NOT_USED 
void
Init_TMP100( void )			
{

	// INIT the TMP100 so that it reads 12 bits of resolution 	
	i2cWaitForIdle();
    i2cPutStartAndWait();       //Write I2C Start Condition, and wait for it to complete
    i2cPutByteAndWait(TMP100_ADDR |I2C_WRITE );  // write addr 

	if(i2cWasAckReceived() )
	{
		i2cPutByteAndWait(0x01);	// Set reg addr to Conf
		i2cPutByteAndWait(0x60); 	// 12 bit mode 
	}
	
	 i2cPutStopAndWait();
}// End of init	 
	

float
Read_TMP100( void )			
{
	BYTE tmpH =0;
	BYTE tmpL=0;
	
	short temp =0;

	i2cWaitForIdle();      //Write I2C Start Condition, and wait for it to complete
    i2cPutStartAndWait();  
    i2cPutByteAndWait(TMP100_ADDR |I2C_WRITE);  // write address
   
    if(i2cWasAckReceived()) 
	{
		i2cPutByteAndWait(0x00);	// Set reg addr to temp register
		i2cRestartAndWait();        //Write I2C Start Condition, and wait for it to complete
    	i2cPutByteAndWait(TMP100_ADDR |I2C_READ );   // read address 
    	if(i2cWasAckReceived()) 
    	{
	    	tmpH = i2cGetByte(I2C_ACK);
	    	tmpL = i2cGetByte(I2C_NACK);
	    }	
	   	    
	    temp = tmpH;
	    temp <<= 8;
	    temp += tmpL;
	    temp>>=4;
	    
	}
	 i2cPutStopAndWait();
	 
	 
	return (float) (temp /16.0) ;
}	
	
	#endif
