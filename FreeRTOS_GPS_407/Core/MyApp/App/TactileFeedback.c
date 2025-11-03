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
char destReached[]= "Waypoint reached"
char course_char;
bool FirstRun = true;

TaskHandle_t hReachWP;
extern TIM_HandleTypeDef htim8;  // PWM timer

/**
 * @brief Functie voor de aansturing van de motors, met een switch case.
 * @param direction = gewenste richting<br>
 * 		  speed_l = snelheid voor linker motor<br>
 * 		  speed_r = snelheid voor rechter motor<br>
 * @return void
 */
void setMotors(int direction, uint16_t speed_l, uint16_t speed_r)
{
    if (speed_l > htim8.Init.Period)
        speed_l = htim8.Init.Period;

    if (speed_r > htim8.Init.Period)
    	speed_r = htim8.Init.Period;

    switch (direction)
    {
        case 1: /// vooruit
            HAL_GPIO_WritePin(M1_1_GPIO_Port, M1_1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(M1_2_GPIO_Port, M1_2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(M2_1_GPIO_Port, M2_1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(M2_2_GPIO_Port, M2_2_Pin, GPIO_PIN_SET);
            logWrite(6, Drive_forward);
            break;

        case -1: /// achteruit
            HAL_GPIO_WritePin(M1_1_GPIO_Port, M1_1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(M1_2_GPIO_Port, M1_2_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(M2_1_GPIO_Port, M2_1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(M2_2_GPIO_Port, M2_2_Pin, GPIO_PIN_RESET);
            logWrite(6, Drive_backward);
            break;

        case 2: /// draai rechts (linker motor vooruit, rechter achteruit)
            HAL_GPIO_WritePin(M1_1_GPIO_Port, M1_1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(M1_2_GPIO_Port, M1_2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(M2_1_GPIO_Port, M2_1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(M2_2_GPIO_Port, M2_2_Pin, GPIO_PIN_RESET);
            logWrite(6, Turn_right);
            break;

        case -2: /// draai links (linker achteruit, rechter vooruit)
            HAL_GPIO_WritePin(M1_1_GPIO_Port, M1_1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(M1_2_GPIO_Port, M1_2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(M2_1_GPIO_Port, M2_1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(M2_2_GPIO_Port, M2_2_Pin, GPIO_PIN_SET);
            logWrite(6, Turn_left);
            break;

        case 3: // stop
            HAL_GPIO_WritePin(M1_1_GPIO_Port, M1_1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(M1_2_GPIO_Port, M1_2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(M2_1_GPIO_Port, M2_1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(M2_2_GPIO_Port, M2_2_Pin, GPIO_PIN_RESET);
            logWrite(6, Stop);
            speed_l = 0;
            speed_r = 0;
            break;

        default:
        	LCD_puts("Driving error");
        	Error_Handler();
    }

    // PWM duty instellen (zelfde op beide motoren hier)
    __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, speed_l);
    __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_4, speed_r);
}

/**
 * @brief Deze functie wordt gebruikt om te kijken of de waypoint bereikt is. Daarnaast wordt hier de heading berekent en wordt de `setMotors()` aan geroepen
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
			ParsedGPS();

			gps_lcd_print = false;

			if (CurrentWaypoint > WaypointCount)
			{
				setMotors(STOP, STANDSTILL, STANDSTILL);
				osDelay(5000);
				continue;
			}

			if (xSemaphoreTake(hGpsDataMutex, portMAX_DELAY) == pdTRUE)
			{
				LCD_clear();
				info = Get_Waypoint_Info(CurrentWaypoint);
				current_speed = parsed_gnrmc.speed;
				CurrentHeading = parsed_gnrmc.course;
				xSemaphoreGive(hGpsDataMutex);
			}


			if (info.distance_m < ARRIVAL_RADIUS_METERS)
			{
				setMotors(STOP, STANDSTILL, STANDSTILL);
				LCD_clear();
				LCD_puts(destReached);
				UART_puts(destReached);
				osDelay(3000);
				CurrentWaypoint++;
			}

			DesiredHeading = heading(CurrentWaypoint);
			desiredheadingValue = DesiredHeading;
			logWrite(4, (void*)&desiredheadingValue);

			if (CurrentHeading < 361.0f && CurrentHeading >= 0.0f)
			{
				float heading_error = DesiredHeading - CurrentHeading;

				if (heading_error > 180.0f)
					heading_error -= 360.0f;

				if (heading_error < -180.0f)
					heading_error += 360.0f;


				if (Uart_debug_out)
					UART_printf(100, "\n\rCurrent heading: %.2f, Desired heading: %.2f, Error: %.2f", CurrentHeading, DesiredHeading, heading_error);

				if (fabs(heading_error) > MAX_HEADING_DIFFERENCE)
				{
					if (heading_error > 0)
					{
						setMotors(RIGHT, SLOW, MEDIUM); //turn right
						LCD_clear();
						LCD_puts("RIGHT");
						UART_puts("\n\n\rRIGHT");
					}

					if (heading_error < 0)
					{
						setMotors(LEFT, MEDIUM, SLOW); //turn left
						LCD_clear();
						LCD_puts("LEFT");
						UART_puts("\n\n\rLEFT");
					}
				}
				else
				{
					setMotors(FORWARD, MEDIUM, MEDIUM); //go foward
					LCD_clear();
					LCD_puts("Foward");
					UART_puts("\n\n\rForward");
				}

			}
			else
			{
				setMotors(FORWARD, MEDIUM, MEDIUM); //go foward
				LCD_clear();
				LCD_puts("FORWARD");
				UART_puts("\n\n\rFORWARD");
			}

			osDelay(200);
		}
		LCD_puts("Final point reached");
		UART_puts("\n\n\rFinal point reached");

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
