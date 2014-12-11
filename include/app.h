#include "board.h"
#include "typedefs.h"
#include "digitdisplay.h"
#include "rtc_driver.h"


typedef enum 
{
	CLOCK_MODE = 0,
	SETTING_MODE
}APP_STATE;


typedef enum
{
	MODE_CHANGE_INPUT_MASK = 0X80,
	NEXT_INPUT_MASK = 0X40,
	SET_INPUT_MASK = 0X20
}MASK_INTR_DATA;


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
