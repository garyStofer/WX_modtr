/**
 * @brief           
 * @file            
 * @author           
 * @dependencies    -
 * @compiler        MPLAB C18 v2.10 or higher <br>
 *                  HITECH PICC-18 V8.35PL3 or higher
 
  
 *********************************************************************/


#ifndef _TCPCLIENT_H_
#define _TCPCLIENT_H_
/////////////////////////////////////////////////
//Global defines

extern void TCP_ClientTask(void);
extern void TCP_ClientInit( void );

#define WX_UPLINK_INTERVAL  15   				// in seconds 
#define AGV_INTERVAL		60					// e.g.  40= 60sec/15sec * 10min

extern BYTE Wind_counts[]; 			// updated by ISR every second  
extern unsigned short Wind_spd;		// in 1/10 Statute Mile  
extern unsigned short Wind_gst;		// in 1/10 Statute Mile 
extern unsigned short Wind_dir;		// in degree 0-359
extern unsigned short Solar_rad;		// estimate of watts per sq meter 
extern unsigned short Baro_Inch;					// Pressure in Kilo Pascal normailzed to sea level
extern unsigned short Temp_F;					// Temp in F
#endif   




