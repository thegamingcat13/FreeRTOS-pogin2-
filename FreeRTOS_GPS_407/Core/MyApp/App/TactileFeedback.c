#include <admin.h>
#include <stdbool.h>
#include "main.h"
#include "gps.h"
#include "cmsis_os.h"
#include <math.h>


int CurrentWaypoint = 20;
float DesiredHeading;
float CurrentHeading;
float MAX_HEADING_DIFFERENCE = 10;
int wpLat;
int wpLon;

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

void GoToDest()
{
	if(fabs(parsed_gnrmc.latitude - wpLat) > 0.00002 && fabs(parsed_gnrmc.longitude - wpLon) > 0.00002)
		drive_foward();
	else if(fabs(parsed_gnrmc.latitude - wpLat) < 0.00002 && fabs(parsed_gnrmc.longitude - wpLon) < 0.00002)
		spin_around();
}

void ReachWaypointTask(void *argument)
{


	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	xTaskNotifyGive(hGpsDataMutex);

	while (TRUE)
	{
		if (xSemaphoreTake(hGpsDataMutex, portMAX_DELAY) == pdTRUE)
		{

			wpLat = returnWaypoints(CurrentWaypoint, 1);
			wpLon = returnWaypoints(CurrentWaypoint, 2);

			if(wpLat > 1 && wpLon > 1) //Check of er iets van data in de Waypoints zit, anders ga door naar de volgende waypoint.
			{
				DesiredHeading = heading();
				//CurrentHeading =

				if(CurrentHeading > DesiredHeading)
					turn_right();
				else if(CurrentHeading < DesiredHeading)
					turn_left();
				else if(fabs(CurrentHeading - DesiredHeading) <  MAX_HEADING_DIFFERENCE)
					GoToDest();

			}
			else
				CurrentWaypoint--;
		}
	}
}



