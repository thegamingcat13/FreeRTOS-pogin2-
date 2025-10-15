
#include "admin.h"
#include <stdbool.h>
#include "cmsis_os.h"
#include "gps.h"


#define MAX_LOGS 100 // Define the maximum number of logs you want to store

Slog logs[MAX_LOGS]; // Array to store log entries

int logCount1 = 0;// Counter to track the number of logs
int logCount2 = 0;// Counter to track the number of logs
int logCount3 = 0;// Counter to track the number of logs
int logCount4 = 0;// Counter to track the number of logs
int logCount5 = 0;// Counter to track the number of logs
int logCount6 = 0;// Counter to track the number of logs


void logWrite(int type, void* info)
{
    // Clear the current log entry (optional, but good practice)
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


void printLogs(void)
{
    // Determine the maximum number of logs across all fields
    int maxLogs = 0;
    if (logCount1 > maxLogs) maxLogs = logCount1;
    if (logCount2 > maxLogs) maxLogs = logCount2;
    if (logCount3 > maxLogs) maxLogs = logCount3;
    if (logCount4 > maxLogs) maxLogs = logCount4;
    if (logCount5 > maxLogs) maxLogs = logCount5;
    if (logCount6 > maxLogs) maxLogs = logCount6;

    // Iterate up to the maximum number of logs
    for (int i = 0; i < maxLogs; i++)
    {
	printf("Log %d:\n", i + 1);

	// Print Latitude if it exists for this index
	if (i < logCount1)
		printf("  Latitude: %s\n", logs[i].latitude);
	 else
		printf("  Latitude: -\n");

	// Print Longitude if it exists for this index
	if (i < logCount2)
		printf("  Longitude: %s\n", logs[i].longitude);
	 else
		printf("  Longitude: -\n");


	// Print Current Heading if it exists for this index
	if (i < logCount3)
		printf("  Current Heading: %s\n", logs[i].heading_current);
	 else
		printf("  Current Heading: -\n");


	// Print Desired Heading if it exists for this index
	if (i < logCount4)
		printf("  Desired Heading: %s\n", logs[i].heading_desired);
	 else
		printf("  Desired Heading: -\n");


	// Print Buttons Pressed if it exists for this index
	if (i < logCount5)
		printf("  Buttons Pressed: %s\n", logs[i].buttons_pressed);
	 else
		printf("  Buttons Pressed: -\n");


	// Print Commands if it exists for this index
	if (i < logCount6)
		printf("  Commands: %s\n", logs[i].commands);
	 else
		printf("  Commands: -\n");


	printf("\n");
	}
}
