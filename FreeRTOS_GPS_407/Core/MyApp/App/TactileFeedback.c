/**
 * @file TactileFeedback.c
 * @brief De functies voor het aansturen van de L298N dual H-bridge driver.
 * En de functies voor de besturing van de motors.
 *
 * @date 28/10/2025
 */

#include <admin.h>
#include <stdbool.h>
#include "main.h"
#include "gps.h"
#include "cmsis_os.h"
#include <math.h>
#include "ultrasonic.h"

int CurrentWaypoint = 0;
int WaypointCount = STRC_AMOUNT;
float DesiredHeading = 0;
float desiredheadingValue;
float CurrentHeading = 0;
float MAX_HEADING_DIFFERENCE = 10;
float wpLat = 0;
float wpLat_temp = 0;
float wpLon = 0;
float current_speed = 0;
char desiredheading[]= "desired_heading";
char currentheading[]= "current_heading";
char Drive_forward[]= "Drive_forward";
char Turn_left[]= "Turn_left";
char Turn_right[]= "Turn_right";
char Drive_backward[] = "Drive_backward";
char Stop[]= "stop";
char motor[]= "motor";
char waypoint[]= "waypoint";
char course_char;
bool FirstRun = true;

TaskHandle_t hReachWP;


/**
 * @brief Functie voor het draaien naar links.
 * @return void
 */
void turn_left()
{
	HAL_GPIO_WritePin(GPIOE, M1_1, SET);
	HAL_GPIO_WritePin(GPIOE, M1_2, RESET);
	HAL_GPIO_WritePin(GPIOE, M2_1, SET);
	HAL_GPIO_WritePin(GPIOE, M2_2, RESET);
	logWrite(6, Turn_left);
}
/**
 * @brief Functie voor het draaien naar rechts.
 * @return void
 */
void turn_right()
{
	HAL_GPIO_WritePin(GPIOE, M1_1, RESET);
	HAL_GPIO_WritePin(GPIOE, M1_2, SET);
	HAL_GPIO_WritePin(GPIOE, M2_1, RESET);
	HAL_GPIO_WritePin(GPIOE, M2_2, SET);
	logWrite(6, Turn_right);
}

/**
 * @brief Functie voor het naar voren rijden.
 * @return void
 */
void drive_forward()
{
	HAL_GPIO_WritePin(M1_1_GPIO_Port, M1_1_Pin, SET);
	HAL_GPIO_WritePin(M1_2_GPIO_Port, M1_2_Pin, RESET);
	HAL_GPIO_WritePin(M2_1_GPIO_Port, M2_1_Pin, RESET);
	HAL_GPIO_WritePin(M2_2_GPIO_Port, M2_2_Pin, SET);
	logWrite(6, Drive_forward);
}
/**
* @brief Functie voor het naar achteren rijden.
* @return void
*/
void drive_backward()
{
	HAL_GPIO_WritePin(M1_1_GPIO_Port, M1_1_Pin, RESET);
	HAL_GPIO_WritePin(M1_2_GPIO_Port, M1_2_Pin, SET);
	HAL_GPIO_WritePin(M2_1_GPIO_Port, M2_1_Pin, SET);
	HAL_GPIO_WritePin(M2_2_GPIO_Port, M2_2_Pin, RESET);
	logWrite(6, Drive_backward);
}
/**
 * @brief Functie voor het stoppen van alle motoren.
 * @return void
 */
void stop()
{
	HAL_GPIO_WritePin(GPIOE, M1_1, RESET);
	HAL_GPIO_WritePin(GPIOE, M1_2, RESET);
	HAL_GPIO_WritePin(GPIOE, M2_1, RESET);
	HAL_GPIO_WritePin(GPIOE, M2_2, RESET);
	logWrite(6, Stop);
}

/**
 * @brief Deze functie wordt gebruikt om te kijken of wij de waypoint bereikt hebben.
 * @param *argument niet gerbuikt.
 * @return void
 */
void ReachWPTask(void *argument)
{
	while (TRUE)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (FirstRun == true)
		{
			while (fabs(wpLat_temp) < 0.0001f)
			{
				WaypointCount --;
				wpLat_temp = returnWaypoints(WaypointCount, 1); // decide how many waypoints we have max of 20
			}
			CurrentWaypoint = 0;
			FirstRun = false;
		}

		while (CurrentWaypoint <= WaypointCount) // Controleer of we nog bezig zijn met een waypoint waar data in zit.
		{
			if (CurrentWaypoint >= WaypointCount)
			{
				stop();
				osDelay(5000);
				continue;
			}


			info = Get_Waypoint_Info(CurrentWaypoint);

			if (info.distance_m < ARRIVAL_RADIUS_METERS)
			{
				stop();
				CurrentWaypoint++;
				osDelay(500);
			}


			if (xSemaphoreTake(hGpsDataMutex, portMAX_DELAY) == pdTRUE)
			{
				current_speed = parsed_gnrmc.speed;
				CurrentHeading = parsed_gnrmc.course;
				xSemaphoreGive(hGpsDataMutex);
			}

			DesiredHeading = heading(CurrentWaypoint);
			desiredheadingValue = DesiredHeading;
			logWrite(4, (void*)&desiredheadingValue);

			if (current_speed > MIN_COG_SPEED)
			{
				float heading_error = DesiredHeading - CurrentHeading;

				if (heading_error > 180.0f) heading_error -= 360.0f;
				if (heading_error < 180.0f) heading_error += 360.0f;

				if (Uart_debug_out)
					UART_printf(100, "\n\rCurrent heading: %.2f, Desired heading: %.2f, Error: %.2f", CurrentHeading, DesiredHeading, heading_error);

				if (fabs(heading_error) > MAX_HEADING_DIFFERENCE)
				{
					if (heading_error > 0)
						turn_right();

					if (heading_error < 0)
						turn_left();
				} else
					drive_forward();
			} else
				drive_forward();

			osDelay(200);
		}
		taskYIELD();
	}
}

/**
 * @brief SkipWaypoint wordt gebruikt om een waypoint over te slaan.
 * Deze functie wordt aangeroepen door de `ARM-keys`.
 * In de functie is een check ingebouwd die kijkt of de functie correct is uitgevoerd.
 * @return 1 = waypoint behaald, verhoog `CurrentWaypoint` met 1 voor volgende waypoint.
 * 0 = waypoint nog niet bereikt.
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
/**
 * @brief BackWaypoint wordt gebruikt om terug te gaan naar de vorige waypoint.
 * Deze functie wordt aangeroepen door de `0ARM-keys`.
 * In de functie is een check ingebouwd die kijkt of de functie correct is uitgevoerd.
 *  * @return 1 = waypoint verlaagt, verlaag `CurrentWaypoint` met 1 voor vorige waypoint.
 * 0 = waypoint niet verlaagt.
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
/**
 * @brief ShowWaypoint wordt gebruikt om te laten zien welk waypoint we naartoe aan het gaan zijn en hoeveel waypoints er totaal zijn.
 * Deze functie wordt aangeroepen door de `ARM-keys`.
 * In de functie is een check ingebouwd die kijkt of de functie correct is uitgevoerd.
 * @return void
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
