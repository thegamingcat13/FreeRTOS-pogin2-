#include <admin.h>
#include <stdbool.h>
#include "main.h"
#include "gps.h"
#include "cmsis_os.h"
#include <math.h>

osThreadId_t hReachWaypointTask;

int CurrentWaypoint = STRC_AMOUNT;
int WaypointCount = STRC_AMOUNT;
float DesiredHeading = 0;
float CurrentHeading = 0;
float MAX_HEADING_DIFFERENCE = 10;
int wpLat = 0;
int wpLon = 0;

void turn_left()
{
	HAL_GPIO_TogglePin(GPIOD, LEDGREEN);
}

void turn_right()
{
	HAL_GPIO_TogglePin(GPIOD, LEDRED);
}

void drive_foward()
{
	HAL_GPIO_TogglePin(GPIOD, LEDORANGE);
}

void spin_around()
{
	HAL_GPIO_TogglePin(GPIOD, LEDORANGE);
	HAL_GPIO_TogglePin(GPIOD, LEDRED);
	HAL_GPIO_TogglePin(GPIOD, LEDGREEN);
	HAL_GPIO_TogglePin(GPIOD, LEDBLUE);
}

void stop()
{
	HAL_GPIO_WritePin(GPIOD, LEDORANGE, RESET);
	HAL_GPIO_WritePin(GPIOD, LEDRED, RESET);
	HAL_GPIO_WritePin(GPIOD, LEDGREEN, RESET);
	HAL_GPIO_WritePin(GPIOD, LEDBLUE, RESET);
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
		spin_around();
		stop();
		CurrentWaypoint ++;
	}

}

void ReachWaypointTask(void *argument)
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



