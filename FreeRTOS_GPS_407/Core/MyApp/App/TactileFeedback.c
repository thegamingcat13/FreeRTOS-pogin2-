#include <admin.h>
#include <stdbool.h>
#include "main.h"
#include "gps.h"
#include "cmsis_os.h"
#include <math.h>

osThreadId_t hReachWPTask;

int CurrentWaypoint = STRC_AMOUNT;
int WaypointCount = STRC_AMOUNT;
float DesiredHeading = 0;
float CurrentHeading = 0;
float MAX_HEADING_DIFFERENCE = 10;
int wpLat = 0;
int wpLon = 0;

void turn_left()
{
	HAL_GPIO_WritePin(GPIOE, M1_1, SET);
	HAL_GPIO_WritePin(GPIOE, M1_2, RESET);
	HAL_GPIO_WritePin(GPIOE, M2_1, RESET);
	HAL_GPIO_WritePin(GPIOE, M2_2, SET);
}

void turn_right()
{
	HAL_GPIO_WritePin(GPIOE, M1_1, RESET);
	HAL_GPIO_WritePin(GPIOE, M1_2, SET);
	HAL_GPIO_WritePin(GPIOE, M2_1, SET);
	HAL_GPIO_WritePin(GPIOE, M2_2, RESET);
}

void drive_foward()
{
	HAL_GPIO_WritePin(GPIOE, M1_1, RESET);
	HAL_GPIO_WritePin(GPIOE, M1_2, SET);
	HAL_GPIO_WritePin(GPIOE, M2_1, RESET);
	HAL_GPIO_WritePin(GPIOE, M2_2, SET);
}
/*
void spin_around()
{
	HAL_GPIO_WritePin(GPIOE, M1_1, RESET);
	HAL_GPIO_WritePin(GPIOE, M1_2, SET);
	HAL_GPIO_WritePin(GPIOE, M2_1, SET);
	HAL_GPIO_WritePin(GPIOE, M2_2, RESET);
}
*/

void stop()
{
	HAL_GPIO_WritePin(GPIOE, M1_1, RESET);
	HAL_GPIO_WritePin(GPIOE, M1_2, RESET);
	HAL_GPIO_WritePin(GPIOE, M2_1, RESET);
	HAL_GPIO_WritePin(GPIOE, M2_2, RESET);
}

void GoToDest()
{
	if(fabs(parsed_gnrmc.latitude - wpLat) > 0.00002 && fabs(parsed_gnrmc.longitude - wpLon) > 0.00002)
	{
		drive_foward();
		osDelay(100);
		xTaskNotifyGive(hParsedGPS);
	}
	else if(fabs(parsed_gnrmc.latitude - wpLat) < 0.00002 && fabs(parsed_gnrmc.longitude - wpLon) < 0.00002)
	{
		//spin_around();
		stop();
		CurrentWaypoint ++;
	}

}

void ReachWPTask(void *argument)
{
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	while (TRUE)
	{
		if (xSemaphoreTake(hGpsDataMutex, portMAX_DELAY) == pdTRUE)
		{
			while (wpLat == 0)
			{
				wpLat = returnWaypoints(WaypointCount, 1);
				WaypointCount --;
				CurrentWaypoint = 0;
			}

			while (CurrentWaypoint <= WaypointCount) //Check of er iets van data in de Waypoints zit, anders ga door naar de volgende waypoint.
			{
				wpLat = returnWaypoints(CurrentWaypoint, 1);
				wpLon = returnWaypoints(CurrentWaypoint, 2);

				DesiredHeading = heading(CurrentWaypoint);
				//CurrentHeading =

				if(CurrentHeading > DesiredHeading)
					turn_right();

				else if(CurrentHeading < DesiredHeading)
					turn_left();

				else if(fabs(CurrentHeading - DesiredHeading) <  MAX_HEADING_DIFFERENCE)
					GoToDest();
			}
		}
	}
}

int SkipWaypoint (void)
{
	int memory = CurrentWaypoint;
	CurrentWaypoint++;
	if (CurrentWaypoint == memory +1)
		return 1;
	else
		return 0;
}

int BackWaypoint (void)
{
	int memory = CurrentWaypoint;
	CurrentWaypoint--;
	if (CurrentWaypoint == memory -1)
		return 1;
	else
		return 0;
}

int ShowWaypoint (void)
{
	int current = CurrentWaypoint;
	char charcurrent = current + '0'; // Convert integer to character
	int total = WaypointCount;
	char chartotal = total + '0'; // Convert integer to character
	LCD_clear();
	LCD_puts("waypoint");
	LCD_puts(charcurrent);
	LCD_puts("of");
	LCD_puts(chartotal);
}
