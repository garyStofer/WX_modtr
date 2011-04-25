#include <string.h>
#include "projdefs.h"
#include "busser1.h"
#include "net\tick.h"
#include "net\helpers.h"
#include "net\tcp.h"
#include "net\dns.h"
#include "net\arp.h"
#include "net\arptsk.h"
#include "rtc.h"
#include "math.h"
#include "WxHTTPclient.h"


extern WORD getTCPMaxDataLenght( void);

//TCP State machine
typedef enum _TCP_CLIENT_STATE {
	SM_HTTP_DNS_WAIT_RESOLVE =0,
	SM_HTTP_DNS_RESOLVED,
	SM_HTTP_SEND_ARP,
	SM_HTTP_WAIT_ARP_RESOLVE     ,
	SM_HTTP_ARP_RESOLVED         ,
	SM_HTTP_CONNECT				, 
	SM_HTTP_WAIT_CONNECTED		,
	SM_HTTP_FINISHED         	
} TCP_CLIENT_STATE;

typedef enum _DECIMALS { NONE=0, ONE, TWO } DECIMALS;
static ROM char const msgURL[] = 	 	"GET /weatherstation/updateweatherstation.php";
static ROM char const msgID[] = 		"?ID="; // i.e. KCA....
static ROM char const msgPWD[] = 		"&PASSWORD="; //
static ROM char const msgACT[] =		"&action=updateraw";
static ROM char const msgDate[] = 	 	"&dateutc=";
static ROM char  const msgTemp[] = 	 	"&tempf=";			// temp in deg F
static ROM char const msgWindDir[] =  	"&winddir=";			// in deg 0-360
static ROM char const msgWindSpd[] =  	"&windspeedmph=";  	// mph -- Statute I presume --  
static ROM char const msgWindGstSpd[]= "&windgustmph=";
static ROM char const msgBaro[]=	 	"&baromin=";		// in inches Mercury
static ROM char const msgSolar[]=	 	"&solarradiation=";	// in Watts per sq meter
//ROM char const msgWingGstSpd10m[]= "&windgustmph_10m=";
//ROM BYTE const msgWingGstDir10m[]= "&windgustdir_10m=300";
//ROM BYTE const msgWingGstDir[]= "&windgustdir=300";
static ROM BYTE const msgDPt[]=			"&dewptf=";			// Relative humidity in %
static ROM BYTE const msgRH[]=		 	"&humidity=";		// Relative humidity in %
//ROM BYTE const msgVis[]=	 	"&visibility=10";		// in NAUTICAL miles ( strange, wind is in mph..)
// additionally the following could be added.
// weather - [text] -- metar style (+RA)
// clouds - [text] -- SKC, FEW, SCT, BKN, OVC

static ROM char const msgHTTP[] = 		" HTTP/1.0\r\n";				// Requierd, Wunderground does not serve 0.9 http-- Do not remove the leading space
static ROM char const msgAccept[] = 	"Accept: text/html\r\n\r\n";	// required plus contains the double NL for termination of HTTP

 
static NODE_INFO tcpServerNode;       //Remote server information
static TCP_CLIENT_STATE smTcp ;
static TCP_SOCKET tcpSocketUser = INVALID_SOCKET;

// Initialize the IP address to Weather Underground.com IP address 


BYTE Wind_counts[WX_UPLINK_INTERVAL]; 		// array to collect the 15 1sec 8 bit count samples between uplinks 

//WORD Wind_gst10Min;
//WORD Wind_gusts[AGV_INTERVAL_10min];
//static BYTE gust_ndx;

//TODO: Could group these weather related vars into one structure 
unsigned short Wind_spd=0;		// in 1/10 Statute Mile  
unsigned short Wind_gst=0;		// in 1/10 Statute Mile 
unsigned short Wind_dir=0;		// in degree 0-359
unsigned short Solar_rad;		// estimate of watts per sq meter 
unsigned short Baro_Inch=2992;  // in 1/100 of an inch 
unsigned short RH=0;		 	// in Relative Humidity in percent
unsigned short RH_10=0;		 	// in Relative Humidity in 1/10 of percent
unsigned short Temp_F =0;		// in 1/10 of deg F
unsigned short T_dewptF=0;		// in 1/10 of deg F

static BYTE baro_avg_smpl =0;
static BYTE baro_avg_ndx = 0;
static float Baro_Avg[AGV_INTERVAL]={0};
#define TEMP_AVG_SMPL 16
static float Temp_Avg[TEMP_AVG_SMPL];
static float RH_Avg[TEMP_AVG_SMPL];
static BYTE tempRH_avg_ndx = 0;
static BYTE tempRH_avg =0;
static WORD Station_elev;
static signed char Temp_cal,Baro_cal,Hyg_cal,Sol_cal, Hyg_cal_offs,Wind_cal;

#ifdef STACK_USE_DNS
IP_ADDR host_addr;
char URLbuff[40];
#endif

void
HTTP_ClientInit( void )
{
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
	
	Temp_cal = appcfgGetc(APPCFG_WX_STATION_TEMP_CAL)-127;
	Baro_cal = appcfgGetc(APPCFG_WX_STATION_BARO_CAL)-127;
	Hyg_cal = appcfgGetc(APPCFG_WX_STATION_HYG_CAL)-127;
	Sol_cal = appcfgGetc(APPCFG_WX_STATION_SOL_CAL)-127;
	Hyg_cal_offs = appcfgGetc(APPCFG_WX_STATION_HYG_CAL_OFFS)-127;
	Wind_cal = appcfgGetc(APPCFG_WX_STATION_WIND_DIR_CAL)-127;

// Setup DNS with URL
// Currently not done since it uses a fair amount of RAM & Prog_rom and EEProm space to hold everything related to the URL and DNS code 
 
// If using DNS to find IP for wunderground
#ifdef STACK_USE_DNS
    strcpypgm2ram(URLbuff, (ROM char *) "weatherstation.wunderground.com");
	serPutRomString( (ROM char *) "\n\rUsing DNS on "); 
	serPutString(URLbuff);
	smTcp = SM_HTTP_DNS_WAIT_RESOLVE;
#else	
	// OR Skip the DNS and go straight to ARP 
	tcpServerNode.IPAddr.v[0] = appcfgGetc(APPCFG_WX_IP_ADDR0);	// 38
	tcpServerNode.IPAddr.v[1] = appcfgGetc(APPCFG_WX_IP_ADDR1); //102
	tcpServerNode.IPAddr.v[2] = appcfgGetc(APPCFG_WX_IP_ADDR2); //136
	tcpServerNode.IPAddr.v[3] = appcfgGetc(APPCFG_WX_IP_ADDR3); //125
	if (tcpServerNode.IPAddr.v[0] ==0)
	{
		serPutRomString( (ROM char *) "\n\rNo IP address for reporting to Wundergroud"); 
		smTcp = SM_HTTP_FINISHED;
	}
	else
	{
		serPutRomString( (ROM char *) "\n\rUsing configured IP address for reporting"); 
		smTcp = SM_HTTP_SEND_ARP;
	}
#endif	
}	
/* Places the string into the TCP put queue as well as the serial output buffer is SerOut flag is set*/
static void 
put_WX_param(ROM char const * pMsg, BOOL SerOut)
{
	BYTE b;
	while ( b = *pMsg++)
	{
		TCPPut(tcpSocketUser, b);
		if (SerOut)
			serPutByte(b);
 	} 
}	 
/* Sends the const message string onto Put_WX_param()then formats argument 'w' according to n decimal points and tacks the 
	formatted result onto the constant string by putting it into the output queues 
*/
static void 
put_WXparam_arg ( ROM char const * pMsg, WORD w, DECIMALS dec_places, BOOL SerOut)
{
	BYTE i,b;
	char strTmp[12]; 	 
   
	
	if ( pMsg != NULL )
	{
		put_WX_param(pMsg, SerOut);
 	}  		
	i=0;
	memset( strTmp,0,12);
// TODO: this conversion to string could be consolidated with the similar code in cmd.c 
	itoa(w,strTmp);
		
	if ( w == 0 && dec_places )
	{
		strTmp[1]='.';
		strTmp[2]='0';	
	}
	else
	{
		i = strlen(strTmp);
		
		if (dec_places ==1)
		{
			strTmp[i] = strTmp[i-1];
			strTmp[i-1] = '.';
		}
		
		if (dec_places ==2)
		{
			strTmp[i] = strTmp[i-1];
			strTmp[i-1] = strTmp[i-2];
			strTmp[i-2]='.';
		}
	}

	// Now tack on the formatted result to the output 	
 	i = 0;
	while ( b = strTmp[i++] ) 
	{
		TCPPut(tcpSocketUser, b);
		if (SerOut)
			serPutByte(b);
	}	

}
/* This function is the HTTP client used to send the weather data out via HTTP 'GET" commands encoded on the URL 
   The state machine goes through the adress resolution protocol ARP to get a  */
//Create a TCP socket for receiving and sending data
void
HTTP_Client(void)
{
    static TICK16 tsecMsgSent;        //Time last message was sent
    char bu[17];
    BYTE b;
    WORD i;
    float tmp;
    float Y;
    float Temp_c;
    
    TRISB_RB6 = 0;		// Make RB6 output -- System LED 		
   
 
    switch (smTcp) 
    {
#ifdef STACK_USE_DNS
	 	case SM_HTTP_DNS_WAIT_RESOLVE:
	    	if (DNSIsResolved( &host_addr))
	    	{
		    	tcpServerNode.IPAddr.v[0] = host_addr.v[0];
		    	tcpServerNode.IPAddr.v[1] = host_addr.v[1];
		    	tcpServerNode.IPAddr.v[2] = host_addr.v[2];
		    	tcpServerNode.IPAddr.v[3] = host_addr.v[3];
		       	smTcp = SM_HTTP_SEND_ARP;
		     }
		    break;
#endif		    	

        case SM_HTTP_SEND_ARP:
            if (ARPIsTxReady()) 
            {	
	           // Clear MAC address first 
                tcpServerNode.MACAddr.v[0] = tcpServerNode.MACAddr.v[1]= tcpServerNode.MACAddr.v[2] = tcpServerNode.MACAddr.v[3] = 0;
			    
			    //Send ARP request for given IP address
                ARPResolve(&tcpServerNode.IPAddr);
                tsecMsgSent = TickGetSec(); 				// take snapshot of time
                smTcp = SM_HTTP_WAIT_ARP_RESOLVE;
            }
                        
            break;

        case SM_HTTP_WAIT_ARP_RESOLVE:
            if (ARPIsResolved(&tcpServerNode.IPAddr, &tcpServerNode.MACAddr)) 
	        {
					serPutRomString( (ROM char *) "\n\rReporting to IP address: ");
					itoa(tcpServerNode.IPAddr.v[0],bu);serPutString(bu);serPutByte(':');
					itoa(tcpServerNode.IPAddr.v[1],bu);serPutString(bu);serPutByte(':');
					itoa(tcpServerNode.IPAddr.v[2],bu);serPutString(bu);serPutByte(':');
					itoa(tcpServerNode.IPAddr.v[3],bu);serPutString(bu);
					
					// Start up via TCP_Finished so that all the reporting data gets initized 
					smTcp = SM_HTTP_FINISHED;  
					tsecMsgSent -= WX_UPLINK_INTERVAL; 	// take snapshot of time and make so that State Finished executes immediatly
            } 
            else
         	{
               	// if after 4 seconds ARP has not resolved, send request again
	            if (TickGetSecDiff(tsecMsgSent) >= (TICK16)4)
	            {
	            	smTcp = SM_HTTP_SEND_ARP;
					LATB6 = 1; 		// indicate no connection via red LED  
					// Note: upon first connect this always fails the first time around -- Not sure why ??
	             }
          	}   	
            
	        break;    

		case SM_HTTP_CONNECT:
              // We can now connect as we have the remote server nodes MAC address
            tcpSocketUser = TCPConnect(&tcpServerNode,80);		// connect on HTTP port
			tsecMsgSent = TickGetSec();
			
               //An error occurred during the TCPListen() function
            if (tcpSocketUser == INVALID_SOCKET) 
            {    
	            smTcp = SM_HTTP_SEND_ARP;		// connection failed, go back to beginning of connection
    			LATB6 = 1; 		// indicate no connection via red LED 
    		}
            else
            {
            	smTcp = SM_HTTP_WAIT_CONNECTED;
            }
    		break;
            
        case SM_HTTP_WAIT_CONNECTED:
        
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

					LATB6 = 0; // clear error indication
	
					GetRTC();
				 	serPutByte('\n');
   			     	serPutByte('\r');    
		            // build HTTP GET request
		            put_WX_param(msgURL, FALSE);	
	            		
					// add in the parameters Wunderground needs 
					put_WX_param(msgID,FALSE);
					
					bu[16] = 0;
					strcpyee2ram(bu, APPCFG_WX_STATIONID, 16); 
					i=0;
					while ( b = bu[i++])
   						TCPPut(tcpSocketUser, b);
  				
  					put_WX_param(msgPWD,FALSE);
					strcpyee2ram(bu, APPCFG_WX_PASSWD, 16); 
					i = 0;
					while ( b = bu[i++])
   						TCPPut(tcpSocketUser, b);
 					
 					put_WX_param(msgACT,FALSE);
					
					// Begin of dateutc			
					put_WX_param(msgDate,FALSE);
					
					put_WXparam_arg( NULL,RTC.Year, NONE,FALSE);
 					TCPPut(tcpSocketUser, '-'); 
					
					put_WXparam_arg( NULL,RTC.Month, NONE,FALSE);
					TCPPut(tcpSocketUser, '-'); 
					
					put_WXparam_arg( NULL,RTC.Date, NONE,FALSE);
					TCPPut(tcpSocketUser, ' '); 

					put_WXparam_arg( NULL,RTC.Hour, NONE,FALSE);
					TCPPut(tcpSocketUser, ':');
					
					put_WXparam_arg( NULL,RTC.Min, NONE,FALSE);
					TCPPut(tcpSocketUser, ':');
					
					put_WXparam_arg( NULL,RTC.Sec, NONE,FALSE);
				    // End of dateutc
		
					// Send the Temperature
					put_WXparam_arg ( msgTemp, Temp_F, ONE,TRUE);
									
					// Send Dew point
					put_WXparam_arg(msgDPt,T_dewptF, ONE,TRUE);
					
					// Send Relative Humidity				
					put_WXparam_arg(msgRH,RH,NONE,TRUE);
	    	
			        // Send the wind speed 
			        put_WXparam_arg( msgWindSpd, Wind_spd, ONE,TRUE);
			        
			       	// Send the Windgust 
					put_WXparam_arg ( msgWindGstSpd, Wind_gst, ONE,TRUE);
					
					// Send the 10Min Windgust speed  --------- Doesn't seem to get recorded by Wunderground 
					//	put_WXparam_arg ( msgWindGstSpd10m, Wind_gst10Min, TRUE);
					
					// Send the Wind Direction 
					put_WXparam_arg ( msgWindDir, Wind_dir, NONE,TRUE);

					
					// Send the Barometer reading, convert to inches Mercury at 0 degC
					put_WXparam_arg ( msgBaro, Baro_Inch, TWO,TRUE);
					
					// Send the solar radiation index in watts/ sq Meter ( estimated) 
					put_WXparam_arg ( msgSolar, Solar_rad, NONE,TRUE);
	
					
				    // Must be HTTP 1.0 request for Wunderground to accept it
					put_WX_param(msgHTTP,FALSE);
				
					// Must have header Accept and double NL to finalize the http protocol package:  
					put_WX_param(msgAccept,FALSE);
   			     
   			    
   			     
   			         //Send contents of transmit buffer, and free buffer for next time around 
                    b =  TCPFlush(tcpSocketUser);
                    tsecMsgSent = TickGetSec();     //Update with current time
  
                   smTcp = SM_HTTP_FINISHED;			// Go to wait for the next update cycle 
                }  
                else // not PUT ready
                {
	              
	            }   
            }
            else 
	        {
		        // if there is no connection after 40 seconds go back and try to establish connection again
		      if (TickGetSecDiff(tsecMsgSent) >= (TICK16)40)
              { 
	               LATB6 = 1; // indicate no connection via red LED
				   LATF0 = 1; // latch error in yellow led 
                   TCPDisconnect(tcpSocketUser);
                   smTcp = SM_HTTP_SEND_ARP;			// Try restarting  the connection 
     	       }
         	}   
            break;



        case SM_HTTP_FINISHED: 		// Finished sending a report, now waiting for the next interval 
              if (TickGetSecDiff(tsecMsgSent) >= WX_UPLINK_INTERVAL)	// every 15 seconds send a new data package 
              { 
// to immediatly affect the next reading by the cal factors changed       
//Temp_cal = appcfgGetc(APPCFG_WX_STATION_TEMP_CAL)-128;
//Baro_cal = appcfgGetc(APPCFG_WX_STATION_BARO_CAL)-128;
//Hyg_cal = appcfgGetc(APPCFG_WX_STATION_HYG_CAL)-128;
//Sol_cal = appcfgGetc(APPCFG_WX_STATION_SOL_CAL)-128;


					
     // The Wind:    
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
#if defined AN_6POLES
	#define AN_CAL_FACTOR 3.75
#elif  defined AN_7POLES
	#define AN_CAL_FACTOR 3.214
#else 
 	#error "Winspeed magnet poles not defined!" 
#endif
			
					Wind_spd = Wind_spd * AN_CAL_FACTOR;				// Averaged wind speed in 1/10 of MPH since last update interval
					Wind_gst = Wind_gst * AN_CAL_FACTOR; 			// Peak wind speed in 1/10 MPH since last update interval

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
              	  	 Wind_dir = (Wind_dir+Wind_cal)%360;	
   	  // The Barometer:	 	
              	  	 	
              	  	 // device sensitiviti is 45mv/kpa, device has 0.2V offset at 15kpa, full range (115kpa) at 0.2v +4.5V = 4.7V
              	  	 // Kpa =  ( (Vout -0.2V) / 0.045v ) + 15kpa
              	  	 // Vout = ADC * 5 / 1024;
              	  	 // 0.2v is 40.92 ADC counts or ~ 41 counts 
              	  	 // (5/1024)/0.045 =0.10861301183 
              	  	 
      	  			// do the math in float Kilo_pascal first and then convert to Inches mercury
              	  	 tmp = (AdcValues[2]-41) * 0.1086130118388182904;
              	  	 tmp += 15;
              	  	 tmp *= 1.0 + 0.0001 * Baro_cal; 		// apply Calibration adjustment - gain  
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
					
// The Solar Radiation:					
					// estimation of solar radiation: Not scientific, not linear, just a guesstimate 
					tmp = AdcValues[3]* 2.8; // estimated watts per Sq Meter, based on max Vout ~1.7V loaded  and roughly 1KW per sq meter max radiation 
	  				tmp *= 1.0 + 0.007 * Sol_cal;
	  				Solar_rad = tmp;
	  				if ( Solar_rad < 10 ) 
	  					Solar_rad =0;
	  					
// The Temperature:
		           // LM134/234 is a current source with a temp coefficient of ~214uV/°K
		           // The current, Iset  is calculated according to (see page5 of data sheet)
		           // Iset = 227.3983e-6 * T°K / R_set (typical) 
		           // °K = Iset*R_set/227.39;
		           // Iset = V_Rload / R_load;
		           // V_Rload = Adc * Vref / 1024 (Vref= 5.0) == ADC * 4.8875e-3
		           // with R_set and R_load at 152O and 9Ko resp, V_Load = 13.4643mv per °K
		           // with Vref at 5V and 10 bit ADC  T°K = ADC * ( 5/1023/13.4643)  == 0.363

           	               	              	 
	                // Get the Temp reading from the external LM334 sensor 
					tmp =  AdcValues[0] * 0.36300; 		// Calculate  °K
					tmp *= 1 + 0.0001 * Temp_cal; 		// apply gain adjustment 
					Temp_c = tmp - 273.15;			 	// Convert to °C
// averaging 					
					Temp_Avg[tempRH_avg_ndx] = Temp_c;
					
					if (tempRH_avg )						// extract average from running average array if we have a full array 
					{
						for ( tmp =0.0,i=0; i < TEMP_AVG_SMPL; i++ )
							tmp += Temp_Avg[i];
						
						Temp_c = tmp/TEMP_AVG_SMPL;
					}
									
// The Humidity:	  					
	  				// Relavive humidity -- Sensor HIH 4030	
	  				// formual:   RH_uncomp = (Vout/Vsup - 0.16 ) / 0.0062
	  				// V_out = VAdc = ADC_Count * Vref/1023
	  				// Vref == Vsup == 5.0 ... Therefore
	  				//	RH_uncomp = (ADC_Count/1024-0.16 )/0.0062 

	  			
	  				tmp = 0.16 * (1.0 -Hyg_cal_offs * 0.002);				// Offset calibration +-24%
	  				tmp = ( AdcValues[4] / 1024.0  - tmp ) / 0.0062; 
	  				tmp = tmp * (1.0 + Hyg_cal * 0.002);						// Gain calibartion  +- 24%
	  				
	  				
	  				// Compensation for temperature 
	  				// RH = RH_uncomp / ( 1.0546 - 0.00216 * Temp_c)	
	  				tmp = tmp /(1.0546 - 0.00216 * Temp_c);
	  				
// Averaging of RH	  				
	  				RH_Avg[tempRH_avg_ndx] = tmp;
	  				
	  				if (++tempRH_avg_ndx >=TEMP_AVG_SMPL )	// array filled, wrap around  and start taking average 
	  				{
		  					tempRH_avg_ndx = 0;
							tempRH_avg = 1;
					}
	  				
	  				if (tempRH_avg )						
					{
						for ( tmp =0.0,i=0; i < TEMP_AVG_SMPL; i++ )
							tmp += RH_Avg[i];
						
						tmp = tmp/TEMP_AVG_SMPL;
					}
	  				
	  				RH_10 = 10*(tmp+0.5); 	// convert to integer in 1/10 of a percent -- for better resolution of the webpage readout 
	  				if (RH_10>1000)			// if uncalibrated limit to 100%  
	  					RH= 1000;
	  					
	  				RH= RH_10/10;
// The Dewpoint: 	  
	// tmp holds RH as float 
					
	// Calculate DewPoint from Temp and humidity
	// simple approximation formula:
	// Td = Temp_c -(100-RH)/5
	/*  				
	  				tmp = (100 - tmp)/5;
	  				tmp = Temp_c - tmp;
	  				T_dewptF  = (tmp*9/5+32)*10;		// convert to 1/10 deg F
	  */			
	  				
	 //Close approximation methode: 
	 /*
	 This expression is based on the August–Roche–Magnus approximation for the saturation vapor pressure of water in air 
	 as a function of temperature. It is considered valid for
     		0 °C < T < 60 °C
     		1% < RH < 100%
     		0 °C < Td < 50 °C 
     
					Formula: 
					a = 17.271
					b = 237.7 °C
					Tc = Temp in Celsius
					Td = Dew point in Celsius
					RH = Relative Humidity
					Y = (a*Tc /(b+Tc)) + ln(RH/100)  
					Td = b * Y / (a - Y)
					*/ 	
					
					 Y = ((17.271 * Temp_c) / (237.7+Temp_c ))+ log(tmp/100);
					 tmp = 237.7 * Y/(17.271-Y);
	  
	  	  			T_dewptF  = (tmp*9/5+32)*10;		// convert to 1/10 deg F
	 				Temp_F =((Temp_c*9/5)+32)*10; // Convert to °F

					if (tcpServerNode.IPAddr.v[0] !=0 )	// if user requested no HTTP reporting to Wunderground
	              		smTcp =SM_HTTP_CONNECT; 		// reconnectd and send next set of parameters
					else
	  					tsecMsgSent = TickGetSec(); 	// just reload the timer and stay in the current state
	     		
              }	  	 
           	 break;
           	 
            
  		default:					// we should never get here !
 	
                LATB6 = 1; 			// Turns on red System LED

 			 break;
       	       		 
	}
        
        
	// read the response from Wunderground -- Needs to be the word "success" 
	if( TCPIsGetReady(tcpSocketUser)  )
	{
// 		static char buff[100];		
//	 	TCPGetArrayChr(tcpSocketUser, (BYTE *)buff, 100,0);
	 
	 	// the response could strech  over multiple packets ! 
		// All we want to know if wunderground reported "success" after a pair of "\r\n" 
		
// Note: It appears that Wunderground always reports success as long as ID and Password is correct, even though the date 
// or the data is misformed or missing. a "success" reply does not mean that the data is actually accepted into Wunderground
// so no evaliation on the response is doen at this time.
//		buff[99] =0;
//	   	serPutString(buff);
	   	TCPDiscard( tcpSocketUser); 
	 } 
}  

      