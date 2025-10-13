#include <admin.h>
#include <stdbool.h>
#include "main.h"
#include "gps.h"
#include "cmsis_os.h"

osThreadId_t hWaypointTask;
osThreadId_t hHeadingTask;

struct waypoints // maakt de struct aan die de latitude en longitude opslaat in floats
{
    float lon;
    float lat;
};

typedef struct waypoints sWaypoints;

sWaypoints Waypoints;

static int current = 0;
static bool filled = false;

sWaypoints aWaypoints[STRC_AMOUNT];
// waypoint
// @brief deze functie checkt of er goede gps-data is binnen gekomen en slaat goede data op in een structure array.
// hiermeer kunnen (STRC_AMOUNT) punten opgslagen worden.
// @return void

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

			txtWriteFloat(longitude, aWaypoints[current].lon);
			txtWriteFloat(latitude, aWaypoints[current].lat);

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

// returnWaypoints
// @brief deze functie kan als debug gebruikt worden.
//Wanneer in de terminal 'W' wordt ingetoetst, worden alle opgeslagen waypoints uitgeprint in de terminal.
// @param pointNumber is de waypoint die moet worden uitgeprint.
// @param type bepaalt of de latitude of longitude wordt uitgeprint.
// @return returnt een float met latitude of longitude waardes afhankelijk van waarde 'type'.



float returnWaypoints (int pointNumber, int type)
{
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

int CurrentWaypointChange (int target)
{
	current = target;
	if (current == target)
		return 1;
	else
		return 0;
}
