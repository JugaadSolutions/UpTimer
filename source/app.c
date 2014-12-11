#include "app.h" 



#define NO_OF_DIGITS	(0X04)

/*
*------------------------------------------------------------------------------
* Public Variables
* Buffer[0] = seconds, Buffer[1] = minutes, Buffer[2] = Hour,
* Buffer[3] = day, Buffer[4] = date, Buffer[5] = month, Buffer[6] = year
*------------------------------------------------------------------------------
*/

UINT8 displayBuffer[8] = {0};
UINT8 readTimeDateBuffer[7] = {0};
UINT8 writeTimeDateBuffer[] = {0X01, 0X27, 0X72, 0X03, 0x027, 0X12, 0X13};
extern BOOL portB_intFlag;
extern UINT8 portB_currentData;


void APP_conversion(void);
void APP_handlePortBInput(void);
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
	BOOL dotOn;

	UINT8 currentState; // stores current state
	UINT8 previousState;
	UINT8 currentNextInput; // stores the value of NEXT digit to be changed
	UINT8 previousNextInput;
	UINT8 currentSetInput; // stores the value of digit to be set
	UINT8 previousSetInput;

	BOOL nextInputValue; // stores input value as 1
	UINT8 blinkIndex;
}APP;

#pragma idata app_data
APP app = {0, 0, 0};
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
	//writeTimeDateBuffer[2] = SetHourMode(0X03,1,1);
	//Set Date and Time


	//WriteRtcTimeAndDate(writeTimeDateBuffer);
	app.state = CLOCK_MODE;

	app.previousState = 0XFF;
	app.previousNextInput = 0XFF;
	app.previousSetInput = 0XFF;
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
	if(portB_intFlag == 1)
	{
		ENTER_CRITICAL_SECTION();  //turn OFF all interrupts
		APP_handlePortBInput();	
		portB_intFlag = 0;
		EXIT_CRITICAL_SECTION();   //turn ON all interrupts
	}
	
	switch ( app.state )	
	{
		case CLOCK_MODE: 
					//UINT8 temp[4] = { '1','3','5','7'};
					ReadRtcTimeAndDate(readTimeDateBuffer);  //Read the data from RTC
					APP_conversion(); // Separate the higher and lower nibble and store it into the display buffer 
					
								
					if( app.dotOn == 0 )
					{
						//DigitDisplay_updateBuffer(temp);		
						DigitDisplay_updateBuffer(displayBuffer);
						app.dotOn = 1;
					}
					else 
					{
						//DigitDisplay_updateBuffer(temp);
						DigitDisplay_updateBuffer(displayBuffer);
						DigitDisplay_DotOn(1, 2);					//Selecting number of displays to display dot 
						app.dotOn = 0;	
					}
		break;
		
		case SETTING_MODE:
					DigitDisplay_blinkOn_ind(500, app.blinkIndex);
					if(app.nextInputValue)						// if next input value = 1
					{
						switch(app.blinkIndex)
						{
							case MINUTES_LSB: 
									displayBuffer[0]++;
									if(displayBuffer[0] > MINUTES_LSB_MAX)
										displayBuffer[0] = '0';
							break;
		
							case MINUTES_MSB:
									displayBuffer[1]++;
									if(displayBuffer[1] > MINUTES_MSB_MAX)
										displayBuffer [1] = '0';
							break;
							case HOURS_LSB: 
									displayBuffer[2]++;
									if(displayBuffer[2] > HOURS_LSB_MAX)
										displayBuffer[2] = '0';
							break;
		
							case HOURS_MSB:
									displayBuffer[3]++;
									if(displayBuffer[3] > HOURS_MSB_MAX)
										displayBuffer[3] = '0';
							break;
										
							default:
							break;
						}				
						
						DigitDisplay_updateBuffer(displayBuffer);
						app.nextInputValue = 0;
					}
					
					break;
	}	
}		


/*
*------------------------------------------------------------------------------
* void APP_handlePortBInput(void)
*
* Summary	: 
*
* Input		: None
*
* Output	: None
*------------------------------------------------------------------------------
*/
void APP_handlePortBInput(void)
{	
	app.currentState = portB_currentData & MODE_CHANGE_INPUT_MASK; // mask the PORTB data to get current state value
	if(app.previousState != app.currentState)			// check for the state change
	{
		if(app.state == CLOCK_MODE)			// if in clock mode
		{
			APP_resetDisplayBuffer();       //reset all digits
			DigitDisplay_updateBuffer(displayBuffer);
			app.nextInputValue = 0;
			app.blinkIndex = 0;				// make blink Index 0
			app.state = SETTING_MODE;		// change app state to clock setting mode
			app.previousNextInput = portB_currentData & NEXT_INPUT_MASK; // mask the PORTB data to get current next input value
			app.previousSetInput = portB_currentData & SET_INPUT_MASK; // mask the PORTB data to get current next input value	
		}
		else
		{
			APP_updateRTC();				//update RTC
			DigitDisplay_blinkOff( );		//blink off
			app.state = CLOCK_MODE;			// change app state to clock mode
		}
			
		app.previousState = app.currentState;		//set the current state value 
	}
	
	if(app.state == SETTING_MODE)	
	{
		app.currentNextInput = portB_currentData & NEXT_INPUT_MASK; // mask the PORTB data to get current next input value
		app.currentSetInput = portB_currentData & SET_INPUT_MASK;	// mask the PORTB data to get current set input value

		 if(app.previousNextInput != app.currentNextInput)	// check for the next input change
		{
			app.nextInputValue = 1;
			app.previousNextInput = app.currentNextInput;
		}
		
		else if(app.previousSetInput != app.currentSetInput)	// check for change in set input
		{
			app.blinkIndex++;
			
			if(app.blinkIndex > 3)
				app.blinkIndex = 0;
			
			app.previousSetInput = app.currentSetInput;
		}
	}
}

void APP_conversion(void)
{
	displayBuffer[0] = (readTimeDateBuffer[1] & 0X0F) + '0';        //Minute LSB
	displayBuffer[1] = ((readTimeDateBuffer[1] & 0XF0) >> 4) + '0'; //Minute MSB
	displayBuffer[2] = (readTimeDateBuffer[2] & 0X0F) + '0';		//Hour LSB
	displayBuffer[3] = ((readTimeDateBuffer[2] & 0X30) >> 4) + '0'; //Hour MSB
}


void APP_resetDisplayBuffer(void)
{
	int i ;
	for(i = 0; i < 4; i++)			//reset all digits
	{
		displayBuffer[i] = '0';
	}
}	



void APP_updateRTC(void)
{
	writeTimeDateBuffer[1] = ((displayBuffer[1] - '0') << 4) | (displayBuffer[0] - '0'); //store minutes
	writeTimeDateBuffer[2] = ((displayBuffer[3] - '0') << 4) | (displayBuffer[2] - '0'); //store Hours

	WriteRtcTimeAndDate(writeTimeDateBuffer);  //update RTC
}		