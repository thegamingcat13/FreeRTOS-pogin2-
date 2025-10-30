/**
* @file UART_keys.c
* @brief Behandelt de communicatie met de UART.<br>
* <b>Demonstreert: IRQ-handling, queue-handling, tasknotifcation, string-handling (strtok, pointers).</b><br>
* Aan de UART is een interrupt gekoppeld, waarvan de ISR (zie: HAL_UART_RxCpltCallback())
* in main.c gegenereerd is. Elke toets (character), die in een terminalprogramma ingedrukt wordt, wordt in de ISR
* in een queue gezet.<br>
* De task UART_keys_IRQ() leest de queue uit nadat het LINEFEED-character is gevonden (nadat ENTER gedrukt is). De
* task leest de queue uit en vult een eigen buffer. Deze buffer wordt via TaskNotify doorgestuurd naar de task die
* deze buffer interpreteert: UART_menu().
* @author MSC
*
* @date 5/5/2022
*/

#include <admin.h>
#include "main.h"
#include "cmsis_os.h"
#include "uart.h"
#include "gps.h"
#include <stdbool.h>
#include "ultrasonic.h"

extern unsigned int os_delay; /// deze waarde kan hier veranderd worden.

/**
* @brief Polt en leest characters in die de gebruiker via Terminalprogramma intikt.
* 1. leest characters van uart
* 2. notifies command-string naar andere task
* @param *argument Niet gebruikt, eventueel een waarde of string om te testen
* @return void
*/
void UART_keys_poll (void *argument)
{
    char buffer[QSIZE_UART];

	osThreadId_t hTask = xTaskGetHandle("UART_menu");

	UART_puts((char *)__func__); UART_puts("started, enter key on terminal\n\r");

	while(TRUE)
    {
	    UART_gets(buffer, QSIZE_UART, TRUE); // wait for string

    	xTaskNotify(hTask, buffer, eSetValueWithOverwrite); // notify task2 with value

		if (Uart_debug_out & UART_DEBUG_OUT)
		{
			UART_puts("\n\r"); UART_puts((char *)__func__);
			UART_puts("- notified with "); UART_puts(buffer);
			UART_puts("\r\n");
		}
	}
}


/**
* @brief Leest characters in die de gebruiker via Terminalprogramma intikt.
* 1. wacht op chars in queue, gevuld door de IRQ_handler (main.c)
* 2. notifies/stuurt command-string naar andere task
* @param *argument Niet gebruikt, eventueel een waarde of string om te testen
* @return void
*/
void UART_keys_IRQ (void *argument)
{
    char  		    buffer[QSIZE_UART];
    char		    buffer_copy[QSIZE_UART];
	int 			pos = 0;
	int             finish = FALSE;
	osThreadId_t    hTask;

	UART_puts((char *)__func__); UART_puts("started, enter key on terminal...\r\n");

	if (!(hTask = xTaskGetHandle("UART_menu")))
		error_HaltOS("Err:UART_hndl 0");

	while(TRUE)
    {
		// laat de VIC de q vullen, die per char in onze eigen string geplaatst wordt
		// tot LFEED gedrukt is of qsize-1 bereikt is.
		// nb: argument 2 is het adres waar ik het q-char in zet
		// nb: q-receive haalt gelijk de buffer leeg (q-peek niet).
		xQueueReceive(hUART_Queue, &buffer[pos], portMAX_DELAY);

		// negeer dit char bij geen data: -1, 255, of CR of spatie
		if (buffer[pos] == 0 || buffer[pos] == -1 || buffer[pos] == 255 || buffer[pos] == CRETURN || buffer[pos] == 32)
			continue;

		//UART_putchar(buffer[pos]);  // echo

		// check of de string gesloten was of gesloten moet worden...
		if (buffer[pos] == LFEED)     // close als LF gedrukt
		{
			buffer[pos] = '\0';
			finish = TRUE;

		}
		else if (pos == QSIZE_UART - 2) // close if end of buf
		{
			buffer[++pos] = '\0';       // first, skip to last position, then close string
			finish = TRUE;
		}

		if (finish)
		{
			// de volgende taak krijgt een copy van de string
			// mijn eigen buffer kan zo gelijk weer gevuld worden door de ISR
			strcpy(buffer_copy, buffer);
			memset(buffer, 0, QSIZE_UART); // clear original buffer
			finish = FALSE;
			pos = 0;

			xTaskNotify(hTask, buffer_copy, eSetValueWithOverwrite); // notify task2 with copy

			if (Uart_debug_out & UART_DEBUG_OUT)
			{
				UART_puts("\n\r"); UART_puts((char *)__func__);
				UART_puts(": UART_menu notified with "); UART_puts(buffer_copy);
				UART_puts("\r\n");
			}
		}
		else
			pos++;
	}
}


/**
* @brief User Interface. De task wacht op kant en klare user-strings (TaskNotifyTake) van ISR-handler, en interpreteert
* de string. Het eerste character geeft het hoofd-commando aan. Afhankelijk van het commando worden opvolgende chars
* geinterpreteerd.
* @param *argument Niet gebruikt, eventueel een waarde of string om te testen
* @return void
*/
void UART_menu (void *argument)
{
	char   *s;
	char   *tok = ",";  // token if command is more than 1 char
	int     val1, val2;

	UART_puts((char *)__func__); UART_puts("started\n\r");

	while (TRUE)
	{
		// wacht op de string; let op de cast van (unsigned long) naar (char *)!
		// want de waarde die ik terug krijg is een pointer.
		s = (char *)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		// check het eerste (en vaak enige) karakter van de inkomende command-string
		// toupper() zorgt ervoor dat het eerste karakter hoofdletter wordt, zo hoef ik niet op kleine
		// letters te checken.
		switch (toupper((unsigned char)s[0]))
		{
		default:  UART_puts(s);
				  UART_puts(" (unkown command)\r\n");
				  break;

		/// <b>0 - 5</b>: Togglet verschillende debug-outputs naar UART
		case '0': Uart_debug_out = (Uart_debug_out ? DEBUG_OUT_NONE : DEBUG_OUT_ALL);
		  	  	  UART_puts("\r\nall debug output = ");
		  	  	  UART_puts(Uart_debug_out == DEBUG_OUT_ALL ? "ON\r\n" : "OFF\r\n");

		  	  	  // als alle output uitgezet wordt, is het handig om gelijk het menu te laten zien.
		  	  	  if (Uart_debug_out == DEBUG_OUT_NONE)
		  	  		  DisplayMenu();
				  break;

		case '1': Uart_debug_out ^= LEDS_DEBUG_OUT; // toggle output on/off
				  UART_puts("\r\nleds output = ");
				  UART_puts(Uart_debug_out & LEDS_DEBUG_OUT ? "ON\r\n" : "OFF\r\n");
				  break;

		case '2': Uart_debug_out ^= ARMKEYS_DEBUG_OUT; // toggle output on/off
		  	      UART_puts("\r\narmkeys output = ");
		  	      UART_puts(Uart_debug_out & ARMKEYS_DEBUG_OUT ? "ON\r\n" : "OFF\r\n");
				  break;

		case '3': Uart_debug_out ^= UART_DEBUG_OUT; // toggle output on/off
		  	      UART_puts("\r\nuart output = ");
		  	      UART_puts(Uart_debug_out & UART_DEBUG_OUT ? "ON\r\n" : "OFF\r\n");
				  break;

		case '4': Uart_debug_out ^= STUDENT_DEBUG_OUT; // toggle output on/off
		  	  	  UART_puts("\r\nstudent output = ");
		  	  	  UART_puts(Uart_debug_out & STUDENT_DEBUG_OUT ? "ON\r\n" : "OFF\r\n");
				  break;

		case '5': Uart_debug_out ^= GPS_DEBUG_OUT; // toggle output on/off
		  	  	  UART_puts("\r\nGPS output = ");
		  	  	  UART_puts(Uart_debug_out & GPS_DEBUG_OUT ? "ON\r\n" : "OFF\r\n");
				  break;

		case '6': Uart_debug_out ^= GPS_DEBUG_RAW_OUT; // toggle output on/off
		  	  	  UART_puts("\r\nGPS-raw output = ");
		  	  	  UART_puts(Uart_debug_out & GPS_DEBUG_RAW_OUT ? "ON\r\n" : "OFF\r\n");
				  break;

		/// ... en reageert ook op een paar letters
		case 'D': /// D: Verandert de Default OSTIME-DELAY, die gebruikt wordt bij de LEDs.
				  /// commando: <b>"d,200"</b> betekent: set delay op 200, NB: spaties worden niet afgevangen...
				  os_delay = atoi(s+2); // skip first 2 characters
				  UART_puts("\r\n os_delay set to: "); UART_putint(os_delay);
				  break;

		case 'M': DisplayMenu(); /// M: Displays het menu (zie my_app.c)
				  break;

		case 'T': CheckTaskData(TRUE); /// T: Displays de stackdata van alle Tasks
				  break;

		case 'P': /// P: Verandert de Prioriteit van een taak
				  /// commando, als: <b>"t,9,20"</b> betekent: set Task 9 op priority 20
				  //  eerst: de 2 waarden worden uit de string gehaald met strtok()
			      //  dan: de strings worden naar int geconverteerd
				  //  nb. dit is wel grof geprogrammeerd zo, in het echt maak je hier een mooie functie van.
			      s = strtok(s,    tok); 				 // naar start van string, negeer 't,'
				  s = strtok(NULL, tok); val1 = atoi(s); // volgende = task_id
				  s = strtok(NULL, tok); val2 = atoi(s); // volgende = priority
				  if (val1 && val2)						 // kleine validiteitscontrole
					  SetTaskPriority(val1, val2);
				  break;

		case 'S': /// S: Start/Stop task
				  /// commando, als: <b>"s,9"</b> betekent: start/stop Task 9
				  //  eerst: de waarde worden uit de string gehaald met strtok()
			      //  dan: de string wordt naar int geconverteerd
				  //  nb. dit is wel grof geprogrammeerd zo, in het echt maak je hier een mooie functie van.
			      s = strtok(s,    tok); 				 // naar start van string, negeer 's,'
				  s = strtok(NULL, tok); val1 = atoi(s); // volgende = task_id
				  if (val1)						 // kleine validiteitscontrole
					  StartStopTask(val1);
				  break;

		case 'W': /// W: Show waypoint data
				  // Eerst worden floats aangemaakt als buffer,
				  // daarna wordt funtie returnWaypoints aangeroepen (route.c) om de floatwaardes op te halen.
				UART_puts("\r\n\nGetting waypoints");
				for (int i = 0; i <STRC_AMOUNT; i++) 			// maakt teller aan om waypoints te printen
				{
					float templat;
					templat = returnWaypoints(i, 1);			// print latitude van punt i
					float templon;
					templon = returnWaypoints(i, 2);			// print longitude van punt i
					UART_printf(100, "\r\n\nWaypoint %d: ", i+1);
					UART_printf(100, "\r\nlon: %f", templat);
					UART_printf(100, "\r\nlat: %f", templon);
				}
				break;

		case 'H': /// H: Debug history output
			printLogs();
			break;

		case 'C': /// C: Verandert huidige waypoint
				  /// commando, als: <b>"c,4"</b> betekent: Set waypoint naar 4
			      //  de waarde word uit de string gehaald met strtok()
		      s = strtok(s,    tok); 				 // naar start van string, negeer 's,'
			  s = strtok(NULL, tok); val1 = atoi(s); // volgende = task_id
			  val1--;

			  int status = CurrentWaypointChange(val1);// status = input user

			  switch (status)
			  {
			  case 1:
				  UART_puts("\n\rAanpassen gelukt");
				  break;
			  case 2:
				  UART_puts("\n\rAanpassing mislukt");
				  break;
			  default: // moet nooit gebeuren
				  UART_puts("\n\rFatal error");
				  break;
			  }

			  break;

		case 'R': /// R: Registreer een waypoint met de huidige gps data
			UART_puts("\n\n\rParsing GPS data");
			ParsedGPS(); // geef de notify aan de struct van gps data
			osDelay(10);
			UART_puts("\n\rSaving waypoints");
			Waypoint();

			switch (status)
			{
			case 1:
				UART_puts("\r\nWaypoint opgeslagen");
				break;
			case 2:
				UART_puts("\r\nError opslaan mislukt Tip:Check gps data (commando G)");
				break;
			default: // moet nooit gebeuren, maar toch
				UART_puts("\r\nUnknown error");
				break;
			}
			break;

		case 'G': /// G: Print de parsed gps structre
			UART_puts("\n\n\rParsed GPS data:");
			UART_printf(100, "\n\r Head: %s", parsed_gnrmc.head);				// 1. header
			UART_printf(100, "\n\r Time: %d", parsed_gnrmc.time);				// 2. hhmmss.sss
			UART_printf(100, "\n\r Status: %s", parsed_gnrmc.status);			// 3. A=valid, V=not valid
			UART_printf(100, "\n\r Latitude: %d", parsed_gnrmc.latitude);		// 4. ddmm.mmmm
			UART_printf(100, "\n\r NS_ind %s", parsed_gnrmc.NS_ind);			// 5. N,S
			UART_printf(100, "\n\r Longitude: %d", parsed_gnrmc.longitude);		// 6. ddmm.mmmm
			UART_printf(100, "\n\r EW_ind %s", parsed_gnrmc.EW_ind);			// 7. E,W
			UART_printf(100, "\n\r Speed: %f", parsed_gnrmc.speed);				// 8. 0.13 knots
			UART_printf(100, "\n\r Course: %f", parsed_gnrmc.course);			// 9. 309.62 degrees
			UART_printf(100, "\n\r Date: %s", parsed_gnrmc.date);				// 10. ddmmyy
			UART_printf(100, "\n\r Mag_var: %f", parsed_gnrmc.mag_var);			// 11. E,W degrees
			UART_printf(100, "\n\r Mag_var_pos : %s", parsed_gnrmc.mag_var_pos);// 12.
			UART_printf(100, "\n\r Mode: %s", parsed_gnrmc.mode);				// 13. A=autonomous, D,E
			UART_printf(100, "\n\r Cs: %s", parsed_gnrmc.cs);					// 14. checkum *34
			break;

		case 'A': /// A: print de gemeten afstand van de sr04 sensor
			if (!sr04_debug)
			{
				UART_puts("\n\n\rEnable sr04 debugging");
				sr04_debug = true;
			}
			else
			{
				sr04_debug = false;
				UART_puts("\n\n\rDisable sr04 debugging");
			}
		}
	}
}
