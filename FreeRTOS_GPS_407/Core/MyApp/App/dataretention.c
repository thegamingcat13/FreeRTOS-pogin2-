/**
 * @file DataRetention.c
 * @brief Onthoudt en slaat de vorige waypoints op, om deze daarna uit te printen op de terminal.
 *
 * De data wordt opgeslagen in `Slog` (zie `main.c` voor de definitie van de struct).
 *
 * @date 15/10/2025
 */
#include "admin.h"
#include <stdbool.h>
#include "cmsis_os.h"
#include "gps.h"
#include "uart.h"
#include "string.h"

//*  Array om log entries op te slaan.*/
Slog logs[MAX_LOGS];

//* Counter om het aantal logs bij te houden.*/
int logCount1 = 0;
int logCount2 = 0;
int logCount3 = 0;
int logCount4 = 0;
int logCount5 = 0;
int logCount6 = 0;

/**
 * @brief slaat alle data op in een structure array. Zoals gps data, motor besturing, knoppen die worden ingedrukt.
 * @param type bepaalt wat voor data het is en in welk deel van de structure array het moet worden opgeslagen.
 * @param info geeft mee welke info er moet worden opgelagen, bijvoorbeeld een string of float-waarde.
 * @return Geen waarde(void).
 */
void logWrite(int type, void* info)
{
    //memset(&logs[logCount], 0, sizeof(Slog));
    switch (type)
    {
        case 1: // Latitude
        	snprintf(logs[logCount1].latitude, sizeof(logs[logCount1].latitude), "%f", returnWaypoints(logCount1, 1));
        	logCount1++;
            break;
        case 2: // Longitude
        	snprintf(logs[logCount2].longitude, sizeof(logs[logCount2].longitude), "%f", returnWaypoints(logCount2, 2));
        	logCount2++;
            break;
        case 3: // Current heading
        	snprintf(logs[logCount3].heading_current, sizeof(logs[logCount3].heading_current), "%.2f", *(float*)info);
        	logCount3++;
            break;
        case 4: // Desired heading
        	snprintf(logs[logCount4].heading_desired, sizeof(logs[logCount4].heading_desired), "%.2f", *(float*)info);
        	logCount4++;
            break;
        case 5: // Buttons pressed
        	strncpy(logs[logCount5].buttons_pressed, (char*)info, sizeof(logs[logCount5].buttons_pressed) - 1);
        	logCount5++;
            break;
        case 6: // Commands
        	strncpy(logs[logCount6].commands, (char*)info, sizeof(logs[logCount6].commands) - 1);
        	logCount6++;
            break;
        default:
            // Handle unknown type (optional)
            break;
    }
}

/**
 * @brief deze functie print alle gegevens die zijn opgelslagen in de Log-structure-arrray.
 * de hoeveelheid lijnen die moeten worden uitgeprint worden automatisch bepaald voor elke logCount.
 * @return Geen waarde(void).
 */
void printLogs(void)
{
    // Determine the maximum number of logs across all fields
    int maxLogs = 0;
    if (logCount1 > maxLogs)
    	maxLogs = logCount1;

    if (logCount2 > maxLogs)
    	maxLogs = logCount2;

    if (logCount3 > maxLogs)
    	maxLogs = logCount3;

    if (logCount4 > maxLogs)
    	maxLogs = logCount4;

    if (logCount5 > maxLogs)
    	maxLogs = logCount5;

    if (logCount6 > maxLogs)
    	maxLogs = logCount6;

    // Iterate up to the maximum number of logs
    for (int i = 0; i < maxLogs; i++)
    {
	UART_printf(100, "Log %d:\n", i+1);

	// Print Latitude if it exists for this index
	if (i < logCount1)
		UART_printf(100, "\r\n  Latitude: %s", logs[i].latitude);
	 else
		UART_printf(100, "  Latitude: -\n");

	// Print Longitude if it exists for this index
	if (i < logCount2)
		UART_printf(100, "\r\n  Longitude: %s", logs[i].longitude);
	 else
		UART_printf(100, "  Longitude: -\n");

	// Print Current Heading if it exists for this index
	if (i < logCount3)
		UART_printf(100, "\r\n  Current Heading: %s", logs[i].heading_current);
	 else
		UART_printf(100, "  Current Heading: -\n");

	// Print Desired Heading if it exists for this index
	if (i < logCount4)
		UART_printf(100, "\r\n  Desired Heading: %s", logs[i].heading_desired);
	 else
		UART_printf(100, "  Desired Heading: -\n");

	// Print Buttons Pressed if it exists for this index
	if (i < logCount5)
		UART_printf(100, "\r\n  Buttons Pressed: %s", logs[i].buttons_pressed);
	 else
		UART_printf(100, "  Buttons Pressed: -\n");

	// Print Commands if it exists for this index
	if (i < logCount6)
		UART_printf(100, "\r\n  Commands: %s", logs[i].commands);
	 else
		UART_printf(100, "  Commands: -\n");


	UART_printf(100, "\r\n");
	}
}
