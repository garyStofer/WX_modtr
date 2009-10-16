#include <string.h>
#include "projdefs.h"
#include "net\tick.h"
#include "net\helpers.h"
#include "net\tcp.h"
#include "net\arp.h"
#include "net\arptsk.h"
#include "rtc.h"
#include "WxHTTPclient.h"


extern WORD getTCPMaxDataLenght( void);

//TCP State machine
typedef enum _TCP_CLIENT_STATE {
	SM_TCP_SEND_ARP =        	0,
	SM_TCP_WAIT_ARP_RESOLVE     ,
	SM_TCP_ARP_RESOLVED         ,
	SM_TCP_CONNECT				, 
	SM_TCP_WAIT_CONNECTED		,
	SM_TCP_FINISHED         	
} TCP_CLIENT_STATE;

typedef enum _DECIMALS { NONE=0, ONE, TWO } DECIMALS;

ROM char const msgURL[] = 	 	"GET /weatherstation/updateweatherstation.php";
ROM char const msgID[] = 		"?ID="; // KCACONCO18
ROM char const msgPWD[] = 		"&PASSWORD="; //weather
ROM char const msgACT[] =		"&action=updateraw";
ROM char const msgDate[] = 	 	"&dateutc=";
ROM char const msgTemp[] = 	 	"&tempf=";			// temp in deg F
ROM char const msgTemp2[] = 	"&temp2f=";			// temp #2  in deg F
ROM char const msgWindDir[] =  	"&winddir=";			// in deg 0-360
ROM char const msgWindSpd[] =  	"&windspeedmph=";  	// mph -- Statute I presume --  
ROM char const msgWindGstSpd[]= "&windgustmph=";
ROM char const msgBaro[]=	 	"&baromin=";		// in inches Mercury
ROM char const msgSolar[]=	 	"&solarradiation=";	// in Watts per sq meter
//ROM char const msgWingGstSpd10m[]= "&windgustmph_10m=";
//ROM BYTE const msgWingGstDir10m[]= "&windgustdir_10m=300";
//ROM BYTE const msgWingGstDir[]= "&windgustdir=300";
//ROM BYTE const msgRH[]=		 	"&humidity=25";		// Relative humidity in %
//ROM BYTE const msgVis[]=	 	"&visibility=10";		// in NAUTICAL miles ( strange, wind is in mph..)
// additionally the following could be added.
// weather - [text] -- metar style (+RA)
// clouds - [text] -- SKC, FEW, SCT, BKN, OVC

ROM char const msgHTTP[] = 		" HTTP/1.0\r\n";				// Requierd, Wunderground does not serve 0.9 http-- Do not remove the leading space
ROM char const msgAccept[] = 	"Accept: text/html\r\n\r\n";	// required plus contains the double NL for termination of HTTP

 
static NODE_INFO tcpServerNode;       //Remote server informatio
static TCP_CLIENT_STATE smTcp = SM_TCP_SEND_ARP;
static TCP_SOCKET tcpSocketUser = INVALID_SOCKET;

// Initialize the IP address to Weather Underground.com IP address 




BYTE Wind_counts[WX_UPLINK_INTERVAL]; 		// array to collect the 15 1sec 8 bit count samples between uplinks 

//WORD Wind_gst10Min;
//WORD Wind_gusts[AGV_INTERVAL_10min];
//static BYTE gust_ndx;

//TODO: Could group these weather related vars into one structure 
unsigned short Wind_spd;		// in 1/10 Statute Mile  
unsigned short Wind_gst;		// in 1/10 Statute Mile 
unsigned short Wind_dir;		// in degree 0-359
unsigned short Solar_rad;		// estimate of watts per sq meter 
unsigned short Baro_Inch;       // in 1/100 of an inch 
unsigned short Temp_F;			// in 1/10 of deg F


static BYTE baro_avg_smpl =0;
static BYTE baro_avg_ndx = 0;
static float Baro_Avg[AGV_INTERVAL]= {0};
static WORD Station_elev;

void
TCP_ClientInit( void )
{
	
	tcpServerNode.IPAddr.v[0] = appcfgGetc(APPCFG_WX_IP_ADDR0);	// 38
	tcpServerNode.IPAddr.v[1] = appcfgGetc(APPCFG_WX_IP_ADDR1); //102
	tcpServerNode.IPAddr.v[2] = appcfgGetc(APPCFG_WX_IP_ADDR2); //136
	tcpServerNode.IPAddr.v[3] = appcfgGetc(APPCFG_WX_IP_ADDR3); //125
	
	smTcp = SM_TCP_SEND_ARP;
	tcpSocketUser = INVALID_SOCKET;
	
	Wind_dir =  Wind_spd = 0;
	Temp_F = 0;

/*	potentially for text to speech output 10 min wind gust peak 
	Wind_gst10Min =Wind_gst = 0
	for ( gust_ndx=0; gust_ndx < AGV_INTERVAL; gust_ndx++ )
	{
		Wind_gusts[gust_ndx]=0;
		
	}
	gust_ndx =0;
*/	

	// Station altitude correction for Sea level pressure at Std atm
	Station_elev = appcfgGetc(APPCFG_WX_STATION_ELEVH);
	Station_elev = Station_elev<<8;
	Station_elev += appcfgGetc(APPCFG_WX_STATION_ELEVL);
	
	
}	
static void 
put_WX_param(ROM char const * pMsg)
{
	BYTE b;
	while ( b = *pMsg++)
   			TCPPut(tcpSocketUser, b);
  
}	 
static void 
put_WXparam_arg ( ROM char const * pMsg, WORD val, DECIMALS dec)
{
	BYTE b;
	char bu[12]; 		// for itoa 
    char *p;			// for strings in memory
    WORD i;
	
	if ( pMsg != NULL )
	{
		put_WX_param(pMsg);
 	}  		

	p = itoa(val,bu);
	
	if (dec == ONE)  // format a decimal point and 1/10 digit
	{
		i = strlen(bu);
		bu[i] = bu[i-1]; // put a decimal point and shift the 1/10 digit one to the right
		bu[i-1] = '.';
		bu[i+1] = 0;
		p = bu;
	}else if ( dec == TWO ) 
	{
		i = strlen(bu);
		bu[i] = bu[i-1]; // put a decimal point and shift the 1/10 digit one to the right
		bu[i-1] = bu[i-2];
		bu[i-2] = '.';
		bu[i+1] = 0;
		p = bu;	
	}	
 	
	while ( b = *p++ ) 
		TCPPut(tcpSocketUser, b);

}
//Create a TCP socket for receiving and sending data
void
TCP_ClientTask(void)
{
    static TICK16 tsecMsgSent;        //Time last message was sent
    char bu[17];
    BYTE b;
    WORD i;
    float tmp;
    	
    switch (smTcp) 
    {
        case SM_TCP_SEND_ARP:
            if (ARPIsTxReady()) 
	        {   // Clear MAC address first 
                tcpServerNode.MACAddr.v[0] = tcpServerNode.MACAddr.v[1]= tcpServerNode.MACAddr.v[2] = tcpServerNode.MACAddr.v[3] = 0;
			    
			    //Send ARP request for given IP address
                ARPResolve(&tcpServerNode.IPAddr);
                
                tsecMsgSent = TickGetSec(); 				// take snapshot of time
                smTcp = SM_TCP_WAIT_ARP_RESOLVE;
            }
            break;

        case SM_TCP_WAIT_ARP_RESOLVE:
            if (ARPIsResolved(&tcpServerNode.IPAddr, &tcpServerNode.MACAddr)) 
	        {
				  // Start up via TCP_Finished so that all the reporting data gets initized 
                  smTcp = SM_TCP_FINISHED;  
            } 
            else
            {	// if after 4 seconds ARP has not resolved, send request again
	            if (TickGetSecDiff(tsecMsgSent) >= (TICK16)4)
	            	smTcp = SM_TCP_SEND_ARP;
            }
	        break;    

		case SM_TCP_CONNECT:
              // We can now connect as we have the remote server nodes MAC address
            tcpSocketUser = TCPConnect(&tcpServerNode,80);		// connect on HTTP port
			tsecMsgSent = TickGetSec();
			
               //An error occurred during the TCPListen() function
            if (tcpSocketUser == INVALID_SOCKET) 
                smTcp = SM_TCP_SEND_ARP;			// connection failed, go back to beginning of connection
            else
            	smTcp = SM_TCP_WAIT_CONNECTED;
           
    		break;
            
        case SM_TCP_WAIT_CONNECTED:
            //Connection has been established
            if (TCPIsConnected(tcpSocketUser)) 
	        {
                 //Checks if there is a transmit buffer ready for accepting data, and that the given socket
                 //is valid (not equal to INVALID_SOCKET for example)
				 if (TCPIsPutReady(tcpSocketUser)) 
	             {
		            
// NOTE : The TCP buffer is only 970 bytes deep. Make sure that you don't overfill the buffer
// while adding more report identifier/data pairs
// No dynamic checking is done for overflow of the 970 bytes 

					GetRTC();
						     
		            // build HTTP GET request
		            put_WX_param(msgURL);	
	            		
					// add in the parameters Wunderground needs 
					put_WX_param(msgID);
					
					bu[16] = 0;
					strcpyee2ram(bu, APPCFG_WX_SATIONID, 16); 
					i=0;
					while ( b = bu[i++])
   						TCPPut(tcpSocketUser, b);
  				
  					put_WX_param(msgPWD);
					strcpyee2ram(bu, APPCFG_WX_PASSWD, 16); 
					i = 0;
					while ( b = bu[i++])
   						TCPPut(tcpSocketUser, b);
 					
 					put_WX_param(msgACT);
					
					// Begin of dateutc			
					put_WX_param(msgDate);
					
					put_WXparam_arg( NULL,RTC.Year, NONE);
					TCPPut(tcpSocketUser, '-'); 
					
					put_WXparam_arg( NULL,RTC.Month, NONE);
					TCPPut(tcpSocketUser, '-'); 
					
					put_WXparam_arg( NULL,RTC.Date, NONE);
					TCPPut(tcpSocketUser, ' '); 

					put_WXparam_arg( NULL,RTC.Hour, NONE);
					TCPPut(tcpSocketUser, ':');
					
					put_WXparam_arg( NULL,RTC.Min, NONE);
					TCPPut(tcpSocketUser, ':');
					
					put_WXparam_arg( NULL,RTC.Sec, NONE);
				    // End of dateutc
		
					// Send the Temperature
					put_WXparam_arg ( msgTemp, Temp_F, ONE);

	     
			        // Send the wind speed 
			        put_WXparam_arg( msgWindSpd, Wind_spd, ONE);
			        
			       	// Send the Windgust 
					put_WXparam_arg ( msgWindGstSpd, Wind_gst, ONE);
					
					// Send the 10Min Windgust speed  --------- Doesn't seem to get recorded by Wunderground 
					//	put_WXparam_arg ( msgWindGstSpd10m, Wind_gst10Min, TRUE);
					
					// Send the Wind Direction 
					put_WXparam_arg ( msgWindDir, Wind_dir, NONE);
					
					// Send the Barometer reading, convert to inches Mercury at 0 degC
					put_WXparam_arg ( msgBaro, Baro_Inch, TWO);
					
					// Send the solar radiation index in watts/ sq Meter ( estimated) 
					put_WXparam_arg ( msgSolar, Solar_rad, NONE);
							
					// Must be HTTP 1.0 request for Wunderground to accept it
					put_WX_param(msgHTTP);
				
					// Must have header Accept and double NL to finalize the http protocol package:  
					put_WX_param(msgAccept);
   			     
   			         //Send contents of transmit buffer, and free buffer for next time around 
                    b =  TCPFlush(tcpSocketUser);
                    tsecMsgSent = TickGetSec();     //Update with current time
  
                   smTcp = SM_TCP_FINISHED;			// Go to wait for the next update cycle 
                }    
            }
            else 
	        {
		        // if there is no connection after 60 seconds go back and try to establish connection again
		      if (TickGetSecDiff(tsecMsgSent) >= (TICK16)60)
              { 
                   TCPDisconnect(tcpSocketUser);
                   smTcp = SM_TCP_SEND_ARP;			// Try restarting  the connection 
                   TRISB_RB6 = 0;LATB6 = 1; 		// indicate no connection via red LED 	
              }
         	}   
            break;



        case SM_TCP_FINISHED: 		// Finished sending a report, now waiting for the next interval 
              if (TickGetSecDiff(tsecMsgSent) >= WX_UPLINK_INTERVAL)	// every 15 seconds send a new data package 
              { 
	                TRISB_RB6 = 0; LATB6 = 0; // clear error indication
	              	
	              	
	                // Get the Temp reading from the external analog sensor 
					tmp =  AdcValues[0] * 0.361;   	// Calculate  deg K
					tmp -= 273.15;				 	// Convert to deg C
					tmp = tmp*9/5;				 	// Convert to deg F
					tmp += 32;
					Temp_F = tmp*10;
					
         
	              	// Davis Cup anemometer conversion to MPH is 2.25 x revolution/sec 
	              	// The wind speed data is aquired as follows:
	              	// The ISR captures counter/Timer3 Low byte once per second and stores the reading
	              	// into the Wind_counts[] on a round robin basis. The array is sized so that it holds all the 1s samples 
	              	// between Update intervals up to Wunderground.com
	              	// When the update interval expires this code itterates through the Wind_counts[] and retrieves the max 
	              	// reading and calculates the averages wind from the 1 sec samples. The average over the update interval is then reported 
	              	// as the current wind and the max as the Windgust for the period. 
	              	// The max value is then stored in a longterm Wind_gusts[] ( it holds readings for the last 10 Mins)
	              	// The Long term Wind_gusts[]is then searched for the highest reading and that is reported as the 10min interval gust
	              		              	
	                           	
	              	// First find the average and the max  of all the 1 second counts between update intervals 
	              	for (Wind_gst = Wind_spd=i=0; i < WX_UPLINK_INTERVAL; i++ )
	              	{ 
		              	b = Wind_counts[i];
		              
		              	if ( b > Wind_gst) 
		              		Wind_gst = b;
		              		
						Wind_spd += b; 
					}
					
					// Calculate the average wind speed without the highest (gust) sample.
					Wind_spd = (Wind_spd - Wind_gst) / (WX_UPLINK_INTERVAL-1);		
					
					// convert to MPH, since the measure window is 1 seconds and there are 6 pulses per revolution 
	              	// the following formual gives MPH in 1/10 miles resolution
	              	// MPH = wind_counts_per_sec  *2.25 /6 *10;
	              	// or MPH = wind_counts_per_sec * 3.75 
					Wind_spd = Wind_spd * 3.75;				// Averaged wind speed in 1/10 of MPH since last update interval
					Wind_gst = Wind_gst * 3.75; 			// Peak wind speed in 1/10 MPH since last update interval

/*					
					// Store current 1s peak in longterm gust array (for 10 min) 
					if (gust_ndx >=AGV_INTERVAL)
						gust_ndx = 0;

						
					Wind_gusts[gust_ndx++]=Wind_gst;
									
					// search for the highest 1s peak in the longtem gust array (highest gust in last 10 min) 
					for ( Wind_gst10Min = i = 0; i < AGV_INTERVAL; i++)
					{
						if ( Wind_gusts[i] > Wind_gst10Min )
							Wind_gst10Min = Wind_gusts[i];
					}
*/	            
	                // The Wind direction 
	              	// the PWM of the windvane does not fully go between 0 and 100 %PWM i.e not fully between DC 0V and DC 5V
	              	// The lowest count from the ADC is 7 when pointig at 0 Deg, and 1017 when pointig at 359.99 deg
              	  	 Wind_dir =  (AdcValues[1]-7)* 360.0 /1010; // calculating degress while compensating for the ~1-99% of the PWM
              	  	 
              	  	 // precautions 
              	  	 if (Wind_dir > 360 ) 
              	  	 	Wind_dir = 360;
              	  	 else if (Wind_dir < 0 ) 
              	  	 	Wind_dir = 0;
   	  	 	
              	  	 	
              	  	 // device sensitiviti is 45mv/kpa, device has 0.2V offset at 15kpa, full range (115kpa) at 0.2v +4.5V = 4.7V
              	  	 // Kpa =  ( (Vout -0.2V) / 0.045v ) + 15kpa
              	  	 // Vout = ADC * 5 / 1023;
              	  	 // 0.2v is 40.92 ADC counts or ~ 41 counts 
              	  	 // (5/1023)/0.045 =0.10861301183 
              	  	 
      	  			// do the math in float Kilo_pascal first and then convert to Inches mercury
              	  	 tmp = (AdcValues[2]-41) * 0.1086130118388182904;
              	  	 tmp += 15;
              	  	 tmp += Station_elev * 3.65625122e-3 ; // pressure drop for elevation in standard atmosphere 
					
				
					// Running average of barometer reading over long periode, i.e. 10 minutes
					Baro_Avg[baro_avg_ndx++] = tmp;
					
					if ( baro_avg_ndx >=AGV_INTERVAL )	// array filled, wrap around  and start taking average 
					{
						baro_avg_ndx = 0;
						baro_avg_smpl = AGV_INTERVAL;
					}
					
					if (baro_avg_smpl )						// extract average from running average array 
					{
						tmp = 0.0;
						for ( i=0; i < baro_avg_smpl; i++ )
						{
							tmp += Baro_Avg[i];
						}	
						tmp /= baro_avg_smpl;
					}	// Else we use the non average baro reading for the first 10 minutes
					
					Baro_Inch = tmp* 29.53;
					
					
					// estimation of solar radiation: Not scientific, not lenar, just a guesstimate 
					Solar_rad = AdcValues[3]* 1.4; // estimated watts per Sq Meter, based on max Vout 3.35V and roughly 1KW per sq meter max radiation 
	  				if ( Solar_rad < 10 ) 
	  					Solar_rad =0;
              		
              		smTcp =SM_TCP_CONNECT; 	// reconnectd and send next set of parameters
              }	  	 
           	 break;
           	 
            
  		default:					// we should never get here !
 				TRISB_RB6 = 0;		// Turn on the RED LED to indicate no connection to Wunderground
                LATB6 = 1; 
 			 break;
       	       		 
	}
        
        
	// read the response from Wunderground -- Needs to be the word "success" 
	if( TCPIsGetReady(tcpSocketUser)  )
	{
	 	//len = TCPGetArray(tcpSocketUser, (BYTE *)buff, 200);
	 	// the response could strech  over multiple packets ! 
		// All we want to know if wunderground reported "success" after a pair of "\r\n" 
		
// Note: It appears that Wunderground always reports success as long as ID and Password is correct, even though the date 
// or the data is misformed or missing. a "success" reply does not mean that the data is actually accepted into Wunderground
// so no evaliation on the response is doen at this time.
	   	
	   	TCPDiscard( tcpSocketUser); 
	 } 
}  

      