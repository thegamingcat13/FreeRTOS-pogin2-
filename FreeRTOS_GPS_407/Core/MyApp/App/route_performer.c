/**
 * @file Route_performer.c
 * @brief Berekent de verschillen tussen de huidige locatie en de waypoint.
 * Dit doet hij met het verschil tussen de lon en lat en de heading te berekenen
 *
 * @date 08/10/2025
 */
#include <admin.h>
#include <stdbool.h>
#include "main.h"
#include "gps.h"
#include "cmsis_os.h"
#include <math.h>

RouteInfo info;   // Struct voor de verschillen
/**
 * @brief Berekent het verschil tussen huidige lon en lat vergeleken met de waypoint.
 * Geeft aan of wij het waypoint hebben berijkt of nog niet.
 * De data wordt vanuit `parsed_grmc` gehaald( zie `gps.h` voor de struct).
 *
 * @param waypoint de struct die lan en lon opslaat.
 * @return info = verschil in lat, lon en de afstand
 */
RouteInfo Get_Waypoint_Info (int waypoint)
{

	float latpoint_deg;    	 // latitude van waypoint
	float lonpoint_deg; 	 // longitude van waypoint
	float latcurrent_deg;    // de huidige latitude
	float loncurrent_deg;    // de huidige longitude

	if (waypoint <= STRC_AMOUNT)
	{

		latpoint_deg = returnWaypoints(waypoint, 1);
		lonpoint_deg = returnWaypoints(waypoint, 2);
		latcurrent_deg = parsed_gnrmc.latitude;
		loncurrent_deg = parsed_gnrmc.longitude;

		info.latdifference = latcurrent_deg - latpoint_deg;
		info.londifference = loncurrent_deg - lonpoint_deg;

		float dx_meters = info.londifference * METER_PER_DEG_LON;
		float dy_meters = info.latdifference * METER_PER_DEG_LAT;
		info.distance_m = sqrtf(dx_meters * dx_meters + dy_meters * dy_meters);

		if (Uart_debug_out) // print de verschillen op de terminal
			{
				UART_puts("\n\r--- Waypoint Info ---");
				UART_printf(100, "\n\rWP %d Target: Lat %.6f, Lon %.6f", waypoint, latpoint_deg, lonpoint_deg);
				UART_printf(100, "\n\rCurrent GPS: Lat %.6f, Lon %.6f", latcurrent_deg, loncurrent_deg);
				UART_printf(100, "\n\rLat Diff: %.6f deg, Lon Diff: %.6f deg", info.latdifference, info.londifference);
				UART_printf(100, "\r\nDistance to WP: %.2f m", info.distance_m);
				UART_puts("\n\r---------------------");
			}

	}
		return (info);
}

/**
 * @brief Berekent de heading eerst in radialen en daarna omgezet in graden.
 * @param waypoint de struct die lan en lon opslaat.
 * @return heading_deg = heading in graden.
 */
float heading (int waypoint)
{

	info = Get_Waypoint_Info(waypoint);

	double overstaande = info.londifference;
	double aanliggende = info.latdifference;

	double heading_rad = atan2f(overstaande, aanliggende);
	double heading_deg = heading_rad * (180.0 / M_PI);

	heading_deg = fmodf((heading_deg +360.0), 360.0);

	if (Uart_debug_out)
		{
			UART_puts("\nHeading:");
			UART_printf(100, "%f", heading_deg);
		}
	return(heading_deg);
}
