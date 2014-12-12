#include "board.h"
#include "typedefs.h"
#include "digitdisplay.h"
#include "rtc_driver.h"


typedef enum 
{
	RESET_STATE = 0,
	COUNT_STATE,
	HALT_STATE
}APP_STATE;

typedef enum 
{
	START_PB = 2,
	STOP_PB = 3,
	RESET_PB
}PB;

typedef enum
{
	MINUTES_LSB = 0,
	MINUTES_MSB,
	HOURS_LSB,
	HOURS_MSB,
	MINUTES_LSB_MAX = '9',
	MINUTES_MSB_MAX = '5',
	HOURS_LSB_MAX = '9',
	HOURS_MSB_MAX = '2'

}TIME_DIGITS;


extern void APP_init(void);
extern void APP_task(void);
