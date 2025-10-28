/**
 * @file Route_performer.c
 * @brief Berekent de verschillen tussen de huidige locatie en de waypoint.
 * Dit doet hij met het verschil tussen de lon en lat en de heading te berekenen
 *
 * @date 08/08/2025
 */
#include <admin.h>
#include <stdbool.h>
#include "main.h"
#include "gps.h"
#include "cmsis_os.h"
#include <math.h>

/**
 * @brief Berekent het verschil tussen huidige lon en lat vergeleken met de waypoint.
 * Geeft aan of wij het waypoint hebben berijkt of nog niet.
 * De data wordt vanuit `parsed_grmc` gehaald( zie `gps.h` voor de struct).
 *
 * @param waypoint de struct die lan en lon opslaat.
 * @return diffs is het verschil in lon en lat.
 */
Differences difference (int waypoint)
{

	float latpoint;    	 // latitude van waypoint
	float lonpoint; 	 // longitude van waypoint
	float latcurrent;    // de huidige latitude
	float loncurrent;    // de huidige longitude
	float latdifference; // het vershil tussen de 2 latitudes
	float londifference; // het verschill tussen de 2 longitudes
	Differences diffs;   // Struct voor de verschillen

	ParsedGPS();
	osDelay(50);

	if (xSemaphoreTake(hGpsDataMutex, portMAX_DELAY) == pdTRUE) // wacht op de semephore
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

			xSemaphoreGive(hGpsDataMutex); // geef de semaphore weer vrij

			if (Uart_debug_out) // print de verschillen op de terminal
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

/**
 * @brief Berekent de heading eerst in radialen en daarna omgezet in graden.
 * @param waypoint de struct die lan en lon opslaat.
 * @return heading_deg de heading in graden.
 */
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
