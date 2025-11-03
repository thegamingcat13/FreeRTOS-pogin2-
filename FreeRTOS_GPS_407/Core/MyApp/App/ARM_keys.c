/**
* @file ARM_keys.c
* @brief Behandelt de communicatie met de ARM-toetsjes met: Eventgroups, TaskNotify, Interrupt-handling.<br>
* <b>Demonstreert: xEventGroupWaitBits(), xTaskGetHandle(), xTaskNotify(), xTaskNotifyWait(),xSemaphoregive(), xSemaphoreTake(). </b><br>
*
* Voert verschillende acties uit voor verschillende arm keys.<br>
* Aan de ARM-keys is een interrupt gekoppeld (zie stm32f4xx_it.c). Die stuurt een event
* door die opgevangen wordt door task ARM_keys_IRQ().
* @author MSC
*
* @date 5/5/2022
*/

#include <admin.h>
#include "main.h"
#include "cmsis_os.h"

// LCD messages
char wplcd[]= "Waypoint opslaan";
char rslcd[]= "Route gestart";
char skiplcd[]= "Waypoint overgeslagen";
char backlcd[]= "Terug naar vorige punt";
char waypoint_skipped[]= "Waypoint_skipped";
char waypoint_back[]= "Waypoint_back";

/**
* @brief Zet een kleurenledje aan en uit.
* @param color De kleur.
* @return void.
*/

void toggle_led (uint32_t color)
{
	HAL_GPIO_TogglePin(GPIOD, color);   // turns led on or off
	osDelay(20);
	HAL_GPIO_TogglePin(GPIOD, color);   // turns led on or off
}


/**
* @brief Deze task handelt de ARM-toets af, die ontvangen is van de ISR-handler (zie: stm32f4xx_it.c).
* @param *argument Niet gebruikt, eventueel een waarde of string om te testen
* @return void.
*/
void ARM_keys_IRQ (void *argument)
{
	unsigned int key;
	osThreadId_t hTask;

	UART_puts("\r\n"); UART_puts((char *)__func__); UART_puts(" started");

	if (!(hTask = xTaskGetHandle("ARM_keys_task")))
		error_HaltOS("Err:ARM_hndle");

    while (1)
	{
		// wait for ISR (EXTI0_IRQHandler()) to signal that a key is pressed
		key = xEventGroupWaitBits(hKEY_Event, 0xffff, pdTRUE, pdFALSE, portMAX_DELAY );

		xTaskNotify(hTask, key, eSetValueWithOverwrite); // notify task2 with value
	}
}


/**
* @brief Task krijgt ARM-key met notificatie binnen, en zet ledjes op die waarde.
* Ook de gekleurde ledjes (behalve blauw, die wordt door de timer gebruikt) krijgen
* een schwung...<br>
* Verschillende acties op basis van de ingedrukte arm key.
* @param *argument Niet gebruikt, eventueel een waarde of string om te testen.
* @return void.
*/
void ARM_keys_task (void *argument)
{
	uint32_t 	 key;
	int			 i, led;

	if (!(hReachWP = xTaskGetHandle("hReachWP")))
		   error_HaltOS("Err:hReachWPTask");

	while(TRUE)
	{
		// WAITING FOR users key
        xTaskNotifyWait (0x00,      		// Don't clear any notification bits on entry.
        		 	 	 0xffffffff, 		// ULONG_MAX, reset the notification value to 0 on exit.
    	                 &key, 				// Notified value.
    	                 portMAX_DELAY);  	// Block indefinitely.

	    xSemaphoreTake(hLED_Sem, portMAX_DELAY); // krijg toegang (mutex) tot leds

    	LED_put((unsigned char)key); // set 8 leds-byte to key-value
	    BUZZER_put (500);
		osDelay(500);

		switch (key)
		{
		case 1: /// Set waypoint
			ParsedGPS();
			LCD_clear();
			LCD_puts(wplcd);
			Waypoint();
			break;

		case 2: /// Start cruising
			xTaskNotifyGive(hReachWP);
			logWrite(5, (void*)started);
			LCD_clear();
			LCD_puts(rslcd);
			break;

		case 3: /// Skip next waypoint
			logWrite(5, (void*)waypoint_skipped);
			SkipWaypoint();
			LCD_clear();
			LCD_puts(skiplcd);
			break;

		case 4: /// Return to previous waypoint
			logWrite(5, (void*)waypoint_back);
			BackWaypoint();
			LCD_clear();
			LCD_puts(backlcd);
			break;

		case 5: /// Show the amount of waypoints and which is next
			ShowWaypoint();
			break;



		}

		if (Uart_debug_out & ARMKEYS_DEBUG_OUT)
		{
			UART_puts("\r\nARM_key pressed set (see leds): "); UART_putint(key);
		}

	    xSemaphoreGive(hLED_Sem); // geef toegang (mutex) vrij

	    // tot slot, laat de gekleurde ledjes meedoen
	    // maar niet blauw, want die is ingezet voor de timer
	    // kijk naar de manier waarop de if-elses er uitzien
		for (i=0; i<3; i++)
		{
			led = (i==0 ? LEDRED : (i==1 ? LEDORANGE : LEDGREEN));
			toggle_led(led);
	  	}
     	taskYIELD(); // done, force context switch
	}
}

