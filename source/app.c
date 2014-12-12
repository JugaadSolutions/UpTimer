#include "app.h" 
#include "linearkeypad.h"
#include "eep.h" 


#define NO_OF_DIGITS	(0X06)
#define EEPROM_STATE_ADDRESS	(0x20)
#define EEPROM_RTC_DATA_ADDRESS (1)
/*
*------------------------------------------------------------------------------
* Public Variables
* Buffer[0] = seconds, Buffer[1] = minutes, Buffer[2] = Hour,
* Buffer[3] = day, Buffer[4] = date, Buffer[5] = month, Buffer[6] = year
*------------------------------------------------------------------------------
*/

//UINT8 readTimeDateBuffer[7] = {0};
UINT8 writeTimeDateBuffer[] = {0X00, 0X00, 0X00, 0X01, 0x01, 0X01, 0X14};


void APP_conversion(void);
void APP_resetDisplayBuffer(void);
void APP_updateRTC(void);

/*
*------------------------------------------------------------------------------
* app - the app structure. 
*------------------------------------------------------------------------------
*/
typedef struct _App
{
	APP_STATE state;
	
	UINT8 displayBuffer[NO_OF_DIGITS];
	UINT8 readTimeDateBuffer[7];
	UINT8 writeTimeDateBuffer[7];

}APP;

#pragma idata app_data
APP app = {0};
#pragma idata


/*
*------------------------------------------------------------------------------
* void APP_init(void)
*
* Summary	: Initialize application
*
* Input		: None
*
* Output	: None
*------------------------------------------------------------------------------
*/
void APP_init( void )
{
	UINT8 i;
	//writeTimeDateBuffer[2] = SetHourMode(0X03,1,1);
	//Set Date and Time

/*	for( i = 0; i < NO_OF_DIGITS+1; i++ )
	{
		writeTimeDateBuffer[i] = 0;
	}
*/

	app.state = Read_b_eep (EEPROM_STATE_ADDRESS); 
	Busy_eep();	

	switch(app.state)
	{
		case RESET_STATE:
			//Turn off Dot
			DOT_CONTROL = 0;

			//Reset Display;
			APP_resetDisplayBuffer( );
			DigitDisplay_updateBuffer(app.displayBuffer);
			
		break;

		case HALT_STATE:
			for( i = 0; i < NO_OF_DIGITS; i++ )
			{
				app.displayBuffer[i] = Read_b_eep(EEPROM_RTC_DATA_ADDRESS+i);
				Busy_eep();	
			}
			DigitDisplay_updateBuffer(app.displayBuffer);
		break;

		default:
		break;
	}

}


/*
*------------------------------------------------------------------------------
* void APP_task(void)
*
* Summary	: 
*
* Input		: None
*
* Output	: None
*------------------------------------------------------------------------------
*/


void APP_task( void )
{
	switch ( app.state )	
	{
		case RESET_STATE:
			if ((LinearKeyPad_getKeyState(START_PB) == 1))
			{
				//Reset RTC ;
				APP_resetDisplayBuffer( );
				WriteRtcTimeAndDate(writeTimeDateBuffer);
	
				//store the state in EEPROM
				Write_b_eep( EEPROM_STATE_ADDRESS , COUNT_STATE );
				Busy_eep( );
				app.state = COUNT_STATE;	
	
				//Turn on Dot
				DOT_CONTROL = 1;
			}
	
		break;
	
		case COUNT_STATE: 
			if ((LinearKeyPad_getKeyState(STOP_PB) == 1))
			{
				UINT8 i;
	
				//Store current data into EEPROM
				for( i = 0; i < NO_OF_DIGITS; i++ )
				{
					Write_b_eep( EEPROM_RTC_DATA_ADDRESS+i , app.displayBuffer[i] );
					Busy_eep( );
				}

				//Change the state
				Write_b_eep( EEPROM_STATE_ADDRESS , HALT_STATE );
				Busy_eep( );
	
				app.state = HALT_STATE;	
				break;
			}
	
			ReadRtcTimeAndDate(app.readTimeDateBuffer);  //Read the data from RTC
			APP_conversion(); // Separate the higher and lower nibble and store it into the display buffer 
			DigitDisplay_updateBuffer(app.displayBuffer); //Write data to display buffer
		break;
	
		case HALT_STATE:
			if ((LinearKeyPad_getKeyState(START_PB) == 1))
			{
				//Reset RTC;
				APP_resetDisplayBuffer( );
				WriteRtcTimeAndDate(writeTimeDateBuffer);
				
				//store the state in EEPROM
				Write_b_eep( EEPROM_STATE_ADDRESS , COUNT_STATE );
				Busy_eep( );
				app.state = COUNT_STATE;	
				break;
			}
	
			if ((LinearKeyPad_getKeyState(RESET_PB) == 1))
			{
				//Reset Display;
				APP_resetDisplayBuffer( );
				DigitDisplay_updateBuffer(app.displayBuffer);;
				
				//store the state in EEPROM
				Write_b_eep( EEPROM_STATE_ADDRESS , RESET_STATE );
				Busy_eep( );
				app.state = RESET_STATE;	

				//Turn off Dot
				DOT_CONTROL = 0;

				break;
			}
	
	
			//DigitDisplay_updateBuffer(app.displayBuffer);
		break;
	
		default:
		break;

	}

}		




void APP_conversion(void)
{
	UINT8 temp = 0;

	//Store the day
	temp = app.readTimeDateBuffer[3];
	temp -= 1;

	//Multiply with max hours
	temp = temp * 24;
	temp += app.readTimeDateBuffer[2];

	app.displayBuffer[0] = (app.readTimeDateBuffer[0] & 0X0F) + '0';        //Seconds LSB
	app.displayBuffer[1] = ((app.readTimeDateBuffer[0] & 0XF0) >> 4) + '0'; //Seconds MSB
	app.displayBuffer[2] = (app.readTimeDateBuffer[1] & 0X0F) + '0';        //Minute LSB
	app.displayBuffer[3] = ((app.readTimeDateBuffer[1] & 0XF0) >> 4) + '0'; //Minute MSB

	app.displayBuffer[4] = (temp%10) + '0';		//Hour LSB
	app.displayBuffer[5] = ((temp/10) >> 4) + '0'; //Hour MSB
}


void APP_resetDisplayBuffer(void)
{
	UINT8 i , temp;
	for(i = 0; i < NO_OF_DIGITS; i++)			//reset all digits
	{
		app.displayBuffer[i] = ' ';
	}
}	

