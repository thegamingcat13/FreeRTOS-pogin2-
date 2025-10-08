#include <admin.h>
#include <stdbool.h>
#include "main.h"
#include "gps.h"
#include "cmsis_os.h"
#include <math.h>

Differences difference (int waypoint)
{

	float latpoint;
	float lonpoint;
	float latcurrent;
	float loncurrent;
	float latdifference;
	float londifference;
	Differences diffs;

	xTaskNotifyGive(hParsedGPS);
	if (xSemaphoreTake(hGpsDataMutex, portMAX_DELAY) == pdTRUE)
	{
		if (waypoint <= STRC_AMOUNT)
		{

			latpoint = returnWaypoints(waypoint, 1);
			lonpoint = returnWaypoints(waypoint, 2);
			latcurrent = parsed_gnrmc.latitude;
			loncurrent = parsed_gnrmc.longitude;

			latdifference = latcurrent - latpoint;
			londifference = loncurrent - lonpoint;
			diffs.latdifference = latdifference;
			diffs.londifference = londifference;

			if (diffs.latdifference <= LAT_PREC && diffs.londifference <= LON_PREC)
				{
					UART_puts("\nWaypoint reached, heading for next point");
					waypoint++;
				}

			xSemaphoreGive(hGpsDataMutex);

			if (Uart_debug_out)
				{
					UART_puts("\nlatitude difference:");
					UART_printf(100, "\r\nlon: %f", latdifference);
					UART_puts("\nlongitude difference:");
					UART_printf(100, "\r\nlon: %f", londifference);
				}

			osDelay(2000);
		}
	}
		return (diffs);
}


float heading (int waypoint)
{
	Differences diffs;

		diffs = difference(waypoint);

		double overstaande = diffs.londifference;
		double aanliggende = diffs.latdifference;

		double heading_rad = atan(overstaande / aanliggende);
		double heading_deg = heading_rad * (180.0 / M_PI);

		if (Uart_debug_out)
			{
				UART_puts("\nHeading:");
				UART_printf(100, "%f", heading_deg);
			}
		return(heading_deg);
}
