#include <admin.h>
#include <stdbool.h>
#include "main.h"
#include "gps.h"
#include "cmsis_os.h"
#include <math.h>

int CurrentWaypoint = 0;
int WaypointCount = STRC_AMOUNT;
float DesiredHeading = 0;
float desiredheadingValue;
float CurrentHeading = 0;
float MAX_HEADING_DIFFERENCE = 10;
int wpLat = 0;
int wpLon = 0;
char desiredheading[]= "desired_heading";
char currentheading[]= "current_heading";
char Drive_forward[]= "Drive_forward";
char Turn_left[]= "Turn_left";
char Turn_right[]= "Turn_right";
char Stop[]= "stop";
char motor[]= "motor";
char waypoint[]= "waypoint";


TaskHandle_t hReachWP;
/*
 * de onderstaande functies worden gebruikt voor het aansturen van de L298N dual H-bridge driver.
 * onderstaande functies kunnen worden opgeroepen om vooruit te rijden of te draaien.
 */
void turn_left()
{
	HAL_GPIO_WritePin(GPIOE, M1_1, SET);
	HAL_GPIO_WritePin(GPIOE, M1_2, RESET);
	HAL_GPIO_WritePin(GPIOE, M2_1, RESET);
	HAL_GPIO_WritePin(GPIOE, M2_2, SET);
	logWrite(6, turn_left);
}

void turn_right()
{
	HAL_GPIO_WritePin(GPIOE, M1_1, RESET);
	HAL_GPIO_WritePin(GPIOE, M1_2, SET);
	HAL_GPIO_WritePin(GPIOE, M2_1, SET);
	HAL_GPIO_WritePin(GPIOE, M2_2, RESET);
	logWrite(6, Turn_right);
}

void drive_foward()
{
	HAL_GPIO_WritePin(GPIOE, M1_1, RESET);
	HAL_GPIO_WritePin(GPIOE, M1_2, SET);
	HAL_GPIO_WritePin(GPIOE, M2_1, RESET);
	HAL_GPIO_WritePin(GPIOE, M2_2, SET);
	logWrite(6, Drive_forward);
}

void stop()
{
	HAL_GPIO_WritePin(GPIOE, M1_1, RESET);
	HAL_GPIO_WritePin(GPIOE, M1_2, RESET);
	HAL_GPIO_WritePin(GPIOE, M2_1, RESET);
	HAL_GPIO_WritePin(GPIOE, M2_2, RESET);
	logWrite(6, Stop);
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


/*
 * GoToDest wordt gebruikt om te bepalen of het voertuig al dicht genoeg bij de waypoint is om door te gaan.
 * Indien het voertuig dicht genoeg erbij is gaan we naar de volgende waypoint.
 */
void GoToDest()
{
	if(fabs(parsed_gnrmc.latitude - wpLat) > LAT_PREC && fabs(parsed_gnrmc.longitude - wpLon) > LON_PREC)
	{ // als de waarden groter zijn dan de define-waardes gaat het voertuig verder vooruit.
		drive_foward();
		osDelay(100);
		ParsedGPS();
		osDelay(50);
	}
	else if(fabs(parsed_gnrmc.latitude - wpLat) < LAT_PREC && fabs(parsed_gnrmc.longitude - wpLon) < LON_PREC)
	{ // als de waarden kleiner zijn dan de define-waardes gaan we door naar het volgende waypoint.
		//spin_around();
		stop(); // waypoint bereikt, stop even
		CurrentWaypoint ++; // ga door naar het volgende waypoint
	}

}

void ReachWPTask(void *argument)
{
	while (TRUE)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (xSemaphoreTake(hGpsDataMutex, portMAX_DELAY) == pdTRUE) // wacht tot de gps-mutex vrij is
		{
			while (wpLat == 0)
			{
				WaypointCount --;
				wpLat = returnWaypoints(WaypointCount, 1);
				CurrentWaypoint = 0;
			}

			while (CurrentWaypoint <= WaypointCount) //Check of er iets van data in de Waypoints zit, anders ga door naar de volgende waypoint.
			{
				wpLat = returnWaypoints(CurrentWaypoint, 1);
				wpLon = returnWaypoints(CurrentWaypoint, 2);
				// vergelijk de heading die aangehouden moet worden met de huidige heading
				DesiredHeading = heading(CurrentWaypoint);
				//logWrite(3, (void*)&heading(CurrentWaypoint));
				desiredheadingValue = DesiredHeading;
				logWrite(4, (void*)&desiredheadingValue);
				// als de headingwaarde kleiner is moet er naar rechts gedraaid worden
				if(CurrentHeading > DesiredHeading)
					turn_right();
				// als de headingwaarde groter is moet er naar links gedraaid worden
				else if(CurrentHeading < DesiredHeading)
					turn_left();
				// als het verschil in heading klein genoeg is kan er naar voren gereden worden en wordt GoToDest opgeroepen
				else if(fabs(CurrentHeading - DesiredHeading) <  MAX_HEADING_DIFFERENCE)
					GoToDest();
			}
			xSemaphoreGive(hGpsDataMutex);
			taskYIELD();
		}
	}
}

/*
 * SkipWaypoint wordt gebruikt om een waypoint over te slaan.
 * Deze functie wordt aangeroepen door de ARM-keys.
 * In de functie is een check ingebouwd die kijkt of de functie correct is uitgevoerd.
 */
int SkipWaypoint (void)
{
	int memory = CurrentWaypoint;
	CurrentWaypoint++;
	if (CurrentWaypoint == memory +1) // verhoog CurrentWaypoint met 1 om zo naar de volgende waypoint te gaan
		return 1;
	else
		return 0;
}
/*
 * BackWaypoint wordt gebruikt om terug te gaan naar de vorige waypoint.
 * Deze functie wordt aangeroepen door de ARM-keys.
 * In de functie is een check ingebouwd die kijkt of de functie correct is uitgevoerd.
 */
int BackWaypoint (void)
{
	int memory = CurrentWaypoint;
	CurrentWaypoint--;
	if (CurrentWaypoint == memory -1) // verlaag CurrentWaypoint met 1 om zo naar de vorige waypoint te gaan
		return 1;
	else
		return 0;
}
/*
 * ShowWaypoint wordt gebruikt om te laten zien welk waypoint we naartoe aan het gaan zijn en hoeveel waypoints er totaal zijn.
 * Deze functie wordt aangeroepen door de ARM-keys.
 * In de functie is een check ingebouwd die kijkt of de functie correct is uitgevoerd.
 */
void ShowWaypoint (void)
{
	int current = CurrentWaypoint;
	char charcurrent[10]; // Converteer integer naar character
	itoa (current, charcurrent, 10);
	int total = WaypointCount;
	char chartotal[10];
	itoa (total, chartotal, 10); // Converteer integer naar character
	LCD_clear();				 // veeg het scherm leeg
	LCD_puts("waypoint");		 // zet de data op het scherm
	LCD_puts(charcurrent);
	LCD_puts("of");
	LCD_puts(chartotal);
}
