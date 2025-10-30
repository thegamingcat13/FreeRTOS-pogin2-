/**
 * @file route.c
 * @brief Functies rondom waypoints: controleren, opslaan en uitlezen van GPS-data.
 * Dit bestand bevat functies om:
 * <li>GPS-data te controleren en op te slaan in een struct-array.</li>
 * <li>Alle opgeslagen waypoints uit te printen.</li>
 * <li>Handmatig de waypoint-counter aan te passen.</li>
 *
 * @date 28/10/2025
 */

#include <admin.h>
#include <stdbool.h>
#include "main.h"
#include "gps.h"
#include "cmsis_os.h"

TaskHandle_t hWaypointTask;

struct waypoints // maakt de struct aan die de latitude en longitude opslaat in floats
{
    float lon;
    float lat;
};

typedef struct waypoints sWaypoints;

sWaypoints Waypoints;

static int current = 0;
static bool filled = false;
bool first_run = true;

sWaypoints aWaypoints[STRC_AMOUNT];
/**
 * @brief deze functie checkt of er goede gps-data is binnen gekomen en slaat goede data op in een structure array.
 * Hiermee kunnen (STRC_AMOUNT) punten opgslagen worden.
 * @return void
*/
char latitude[]= "latitude";
char longitude[]= "longitude";

void Waypoint ()
{
    if (xSemaphoreTake(hGpsDataMutex, portMAX_DELAY) == pdTRUE) // wacht tot de semaphore van de gps-data vrij is
    {
		if (parsed_gnrmc.status == 'A') 						// check of de gps-data die ontvangen is goed is
		{
			aWaypoints[current].lon = parsed_gnrmc.longitude;	// kopieer de latitude van de gps-data naar de waypoints-struct
			aWaypoints[current].lat = parsed_gnrmc.latitude;	// kopieer de longitude van de gps-data naar de waypoints-struct

			logWrite(1, 0);										// sla de latitude op in de terminal-log
			logWrite(2, 0);										// sla de longitude op in de terminal-log

			current++;
			if (current >= STRC_AMOUNT)							// als er 20 sets data zijn opgeslagen, zet de pointer weer terug op de eerste set
			{
				filled = true;
				current = 0;
			}
		}

		xSemaphoreGive(hGpsDataMutex);							// geef de mutex weer vrij
    }
}

/**
 * @brief Wanneer in de terminal 'W' wordt ingetoetst, worden alle opgeslagen waypoints uitgeprint in de terminal.
 * Deze functie kan als debug gebruikt worden.
 * @param PointNumber is de waypoint die moet worden uitgeprint.
 * @param Type 1 = latitude, 2 = longitude
 * @return Een float met latitude of longitude waardes afhankelijk van waarde 'type'.
 */
float returnWaypoints (int pointNumber, int type)
{
	if (first_run)
	{
		for (int i = 0; i <= 19; i++)
		{
			aWaypoints[i].lat = 0;
			aWaypoints[i].lon = 0;
		}
		first_run = false;
	}
	switch (type)
	{
		case 1: //
				float lat;
				lat = aWaypoints[pointNumber].lat;
				return lat;
				break;

		case 2: //
				float lon;
				lon = aWaypoints[pointNumber].lon;
				return lon;
				break;

		default: //
				return 475.39182; //if we see this number in the terminal we know that something is verrrryyyyyy wrong. Should not happen :)
				break;
	}
}

/**
 * @brief Wijzigt handmatig de huidige waypoint.
 * @param target Het waypointnummer waar naartoe gewisseld moet worden.
 * @return int 1 als de wijziging gelukt is, 0 als niet.
 */
int CurrentWaypointChange (int target)
{
	current = target;
	if (current == target)
		return 1; // als het aanpassen gelukt is, return 1
	else
		return 0; // als het aanpassen mislukt is, return 0
}
