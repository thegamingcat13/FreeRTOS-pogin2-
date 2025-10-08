/**
* @file admin.c
* @brief Bestaat uit 3 onderdelen: <br>
* Deel 1. <b>Initialisatieoutput naar uart en lcd-scherm</b>.<br>
* Deel 2. De functies voor de <b>initialisatie van de handles</b>. <br>
* Deel 3. De functies voor opzet en <b>handling van tasks</b>.
* Dit deel bevat de functies die tasks aanmaakt én hulpfuncties voor tasks.<br>
* <b>Demonstreert: osThreadNew(), vTaskSuspendAll(), xTaskResumeAll(), uxTaskGetStackHighWaterMark(),
* vTaskPrioritySet() en c-stuff als array-handling, structures,</b> en let op:
* het gebruiken van specifieke structure-members binnen andere structures, zie tasks[]!<br>
*
* Elke task wordt op dezelfde manier aangemaakt, met osThreadNew() die een trits
* aan argumenten wil, maw: de argumenten zijn de variabelen. Dus waarom niet per task
* de variabelen op een rijtje zetten in een array - zo krijg je veel meer overzicht.
* Eerst wordt een structure aangemaakt, die de argumenten gaat bevatten, struct TaskData.
*
* TaskData bevat 4 members: func (de taskname), argument (eventueel argument voor task, nu niet gebruikt),
* struct attr (FreeRTOS structure, waarvan we enkele gebruiken!), de taskhandle.
* In de array tasks[] worden alleen die members van TaskData geinitialiseerd die nodig zijn om
* elke task/thread aan te maken. <br>Dat zijn: 1. de task, 2. het argument voor de task, 3. de tasknaam
* als string, 4. de stack size, 5. de prioriteit.
* De members 3, 4 en 5 zijn members van de attr structure.
*
* @author MSC
* @date 5/5/2022
*/

#include "main.h"
#include "cmsis_os.h"
#include "task.h"
#include "admin.h"

/// output strings for initialization
char *app_name    = "\r\n=== freeRTOS_GPS 407 ===\r\n";
char *app_nameLCD = "freeRTOS_GPS 407"; // max 16 chars

/// default: debug all output to uart
int Uart_debug_out = DEBUG_OUT_NONE;


/// all handles used, note: defined to 'extern' in admin.h
QueueHandle_t 	      hKey_Queue;
QueueHandle_t 	      hUART_Queue; /// uses UART2
QueueHandle_t 	      hGPS_Queue;  /// uses UART1
SemaphoreHandle_t     hLED_Sem;
SemaphoreHandle_t	  hGpsDataMutex;
EventGroupHandle_t 	  hKEY_Event;
TimerHandle_t         hTimer1;



/** tasks[] is een array van structures met alleen de argumenten om een taak aan te maken.
 *
 * - **NB:**<br>
 * 	- Dit wist je vast niet. De members .attr.name en .attr.stack_size etc. zijn enkele members van de struct osThreadAttr_t;
 * enkele daarvan zijn nodig om een taak te starten en dus alleen die heb ik in deze array geinitialiseerd - dat kan
 * dus op de manier die je hier ziet, dus met een . (punt) ervoor.
 * 	- Als je in de tweede kolom ipv NULL "hallo" zet, wordt "hallo" aan de taak meegegeven - kun je evt gebruiken voor
 * testen of iets dergelijks.
 * 	- De prioriteiten worden hier met default-waarden gezet; hier kun je zelf mee spelen. Via de UART kun je elke
 * taak een andere prioriteit geven. Kun je ook mee spelen. Trouwens, als je taken dezelfde prioriteit
 * geeft, dan gaat het OS ze met Round Robin keurig allemaal gelijk verdelen.
 * 	- De laatste regel bevat nullen; deze regel wordt gebruikt om for-loepjes eenvoudig te laten eindigen.
 * 	- Om een taak niet te laten starten, zet gewoon '//' voor de regel en Bob is je oom (Bob is your uncle)!
 *
 * - **Doel van de array:**
 * 	-# makkelijk en overzichtelijk om taken te starten
 * 	-# geeft een overview van alle taken en argumenten (name, stacksize, prty's)
 */
TASKDATA tasks[] =
{
// function      arg   name                        stacksize (* 4 = 32bit)     priority
// ----------------------------------------------------------------------------------------------------------------------------
  // in ARM_keys.c
{ ARM_keys_IRQ, NULL, .attr.name = "ARM_keys_IRQ", .attr.stack_size = 450, .attr.priority = osPriorityNormal1 },
{ ARM_keys_task,NULL, .attr.name = "ARM_keys_task",.attr.stack_size = 450, .attr.priority = osPriorityNormal2 },

  // UART_keys.c
{ UART_keys_IRQ,NULL, .attr.name = "UART_keys_IRQ",.attr.stack_size = 550, .attr.priority = osPriorityBelowNormal4 },
{ UART_menu,    NULL, .attr.name = "UART_menu",    .attr.stack_size = 2300, .attr.priority = osPriorityBelowNormal6 },

  // gps.c
{ GPS_getNMEA,  NULL, .attr.name = "GPS_getNMEA",  .attr.stack_size = 1200, .attr.priority = osPriorityBelowNormal1 },
{ PARSED_GPS,   NULL, .attr.name = "PARSED_GPS",   .attr.stack_size = 1200, .attr.priority = osPriorityBelowNormal1 },

  // student.c
{ Student_task1,NULL, .attr.name = "Student_task1",.attr.stack_size = 700, .attr.priority = osPriorityBelowNormal7 },

  // ledjes.c
  // NOTE: ledtasks 1 & 2 moeten dezelfde priority hebben, anders 'sterft' de taak met de laagste priority
  //       wat wel kan: afdwingen dat taken aan de beurt komen door notifications, zie ledasks 3 & 4
{ LED_Task1,    NULL, .attr.name = "LED_Task1",    .attr.stack_size = 400, .attr.priority = osPriorityBelowNormal4 },
{ LED_Task2,    NULL, .attr.name = "LED_Task2",    .attr.stack_size = 400, .attr.priority = osPriorityBelowNormal4 },
{ LED_Task3,    NULL, .attr.name = "LED_Task3",    .attr.stack_size = 400, .attr.priority = osPriorityBelowNormal5 },
{ LED_Task4,    NULL, .attr.name = "LED_Task4",    .attr.stack_size = 400, .attr.priority = osPriorityBelowNormal4 },

  // deze laatste niet wissen, wordt gebruik als 'terminator' in for-loops
{ NULL,         NULL, .attr.name = NULL,           .attr.stack_size = 0,       .attr.priority = 0 }
};


/**
 *************************************************************************************************************************
 *************************************************************************************************************************
 * @remark DEEL 1 van admin.c
 * Bevat de functies voor de initialisatie output, vooral naar UART, zoals het 'menu'
 *************************************************************************************************************************
 *************************************************************************************************************************
 */

/**
* Schrijft applicatiedata (versie, werking, menu) naar UART.
* Les: je ziet hoe je handig een lange string kunt opstellen met het '\'-karakter.
* @return void
* @author MSC
*/
void DisplayVersion(void)
{
	char infobuf[100];
    osVersion_t osv;

    char *functionality =
"Deze applicatie laat verschillende FreeRTOS-mechanismen zien, zoals \r\n\
queues, notifications, eventgroups, mutex-semaphores, message-buffers, interrupt handling \r\n\
en hoe je makkelijk tasks aanmaakt (admin.c) en hoe je de bidirectionele omgang \r\n\
met de UART-comport gebruikt.\r\n\
Zie verder de Doxygen documentatie van de applicatie.\r\n\
Michiel Scager (update: april 2023)\r\n";

	LCD_clear();
	LCD_puts(app_nameLCD);

	UART_puts(app_name);
	UART_puts(functionality);

 	osKernelGetInfo(&osv, infobuf, sizeof(infobuf));
	UART_puts("\t-----");
	UART_puts("\r\n\t");                       UART_puts(infobuf);
	UART_puts("\r\n\tVersion: ");              UART_putint(osv.kernel);
	UART_puts("\r\n\tOS Timer freq (MHz): " ); UART_putint(osKernelGetSysTimerFreq()/1000000);
	UART_puts("\r\n\tOS Tick freq (Hz): " );   UART_putint(osKernelGetTickFreq());
	UART_puts("\r\n\t-----\r\n");

    DisplayMenu();
}


/**
* User Interface (menu) naar UART.
* @return void
* @author MSC
*/
void DisplayMenu(void)
{
	char *menu =
"\r\n\
Menu:===============================================================\r\n\
key: function\r\n\
 0 : [on/off] ALL TEST OUTPUT\r\n\
 1 : [on/off] LEDS output\r\n\
 2 : [on/off] ARM_keys output\r\n\
 3 : [on/off] UART_keys output\r\n\
 4 : [on/off] STUDENT output\r\n\
 5 : [on/off] GPS output\r\n\
 6 : [on/off] GPS raw data output\r\n\
 d : change DELAY time (default 200), eg. 'd,50'\r\n\
 p : change TASK PRIORITY, eg. 'p,7,20' sets priority of task 7 to 20\r\n\
 t : display TASK DATA (number, priority, stack usage, status)\r\n\
 s : start/stop TASK, eg. s,7 starts or stops task 7\r\n\
=====================================================================\r\n";

    UART_puts(menu);
}


/**
 *************************************************************************************************************************
 *************************************************************************************************************************
 * @remark DEEL 2 van admin.c
 * Bevat de functies voor het creëren van alle handles
 *************************************************************************************************************************
 *************************************************************************************************************************
 */


/**
* @brief Fatale OS-error opgetreden tijdens startup of runtime, doorgaan is
* zinloos.
* Het OS wordt tot stoppen gebracht, er wordt output gegenereerd waar mogelijk,
* dus op UART en LCD.
* @param msg Foutmelding
* @return void
*/
void error_HaltOS(char *msg)
{
	LCD_puts(msg);
	UART_puts(msg); UART_puts(". Application halted\r\n");

	BUZZER_put(1000);
	vTaskSuspendAll(); // stop alle tasks

	while (TRUE)
	{
	}
}


/**
* @brief Creates alle handles voor deze applicatie
* @return void
*/
void CreateHandles(void)
{
	if (!(hLED_Sem = xSemaphoreCreateMutex()))
		error_HaltOS("Error hLED_Sem");

	if (!(hGpsDataMutex = xSemaphoreCreateMutex()))
		error_HaltOS("Error GPS_Sem");

	if (!(hUART_Queue = xQueueCreate(QSIZE_UART, sizeof(unsigned int))))
		error_HaltOS("Error hUART_Q");

	if (!(hGPS_Queue = xQueueCreate(GPS_MAXLEN, sizeof(unsigned char))))
		error_HaltOS("Error hGPS_Q");

	if (!(hKEY_Event = xEventGroupCreate()))
		error_HaltOS("Error hLCD_Event");

	if (!(hTimer1 = xTimerCreate("Timer_1", pdMS_TO_TICKS(TIMER1_DELAY), pdTRUE, 0, (TimerCallbackFunction_t)Timer1_Handler)))
		error_HaltOS("Error hTimer1");

	UART_puts("\n\rAll handles created successfully.");

	UART_puts("\n\rTimer set to: ");
	UART_putint((int)TIMER1_DELAY); // (int)-cast is nodig!
	xTimerStart(hTimer1, 0); // start de timer...
}

/**
* @brief Vangt de FreeRTOS software-interrupt op en toggelt een ledje
* @param hTimer1 De handle van de timer
* @return void
*/
void Timer1_Handler(void)
{
	HAL_GPIO_TogglePin(GPIOD, LEDBLUE);   // turns led on/off
	CheckTaskData(FALSE); // checktaskdate without output to terminal
}




/**
 *************************************************************************************************************************
 *************************************************************************************************************************
 * @remark DEEL 3 van admin.c
 * Bevat de functies voor het creëren van tasks en admin-functies voor tasks
 *************************************************************************************************************************
 *************************************************************************************************************************
 */



/**
* @brief Creëert alle taken door te scannen door de TASKDATA-argumenten-array.<br>
* Let op. Eerst wordt de scheduler gestopt, dan de taken gecreëerd, dan de scheduler gestopt.
* Waarom? Anders gaan taken gelijk beginnen en wachten op taken die nog niet bestaan, dat geeft een rommeltje.
* @return void
*/
void CreateTasks(void)
{
	PTASKDATA ptd = tasks; // tasks == &tasks[0]: both are addresses of first struct
	vTaskSuspendAll(); // zet de scheduler stil en dwing FreeRTOS om eerst alle taken te starten

	for (; ptd->func != NULL; ptd++) // loopje zonder teller; gewoon de pointer steeds ophogen...
	{
		UART_puts("\r\nCreated: ");
		UART_puts((char *)ptd->attr.name);

		// nb. kijk naar de haakjes van argument 3: het gaat om het adres van (structure attr).
		ptd->hTask = osThreadNew(ptd->func, ptd->argument, &(ptd->attr));
	}

	xTaskResumeAll();   // start nu de scheduler: play ball
	CheckTaskData(TRUE);  // display alle taskdata op UART with output to terminal
}


/**
* @brief Zoekt in tasks-array de task, en returns de handle.
* Deze handle wordt door CreateTasks() in de array geplaatst. In plaats van deze functie kan ook
* de OS-functie xTaskGetHandle() gebruikt worden. Alleen is 'mijn' functie sneller.
* @param *taskname De tasknaam
* @return osThreadId_t De taskhandle
*/
osThreadId_t GetTaskhandle(char *taskname)
{
	PTASKDATA ptd = tasks;

	for (; ptd->func != NULL; ptd++)
		if (!strcmp(taskname, ptd->attr.name))
			return (ptd->hTask);

	return (NULL); // if task was not found...
}


/**
* @brief Verandert de prioriteit van een taak.
* Eerst wordt de taak gezocht in de lijst van alle taken (tasks[])
* Daarna wordt de priority in de lijst gewijzigd en prioriteit van de taak aangepast. De functie
* DisplayTaskData() toont op de UART het tasknummer dat je nodig hebt.
* @param task_id Task identifier
* @param new_priority Nieuwe prioriteit
* @return void
*/
void SetTaskPriority(int task_id, int new_priority)
{
	PTASKDATA ptd = tasks;
	unsigned int 	task_nr;


	for (task_nr = 1; ptd->func != NULL; ptd++, task_nr++)
		if (task_nr == task_id) // hebbes
		{
			ptd->attr.priority = new_priority;			// struct-member
			vTaskPrioritySet(ptd->hTask, new_priority); // taskpriority
			// of
			// osThreadSetPriority(ptd->hTask, new_priority);

			UART_puts("\r\nSet task "); UART_puts(ptd->attr.name);
			UART_puts("'s priority to: "); UART_putint(new_priority);
			return;
		}
}


/**
* @brief Suspends (stopt) of Resumes (start) een task.
* Pas op. Als een task suspended wordt als hij een mutex bezet houdt blokkeer
* je alle tasks die op die mutex wachten. En als je de task stopt die de UART leest,
* schiet je jezelf ook in de voet.
* @return void
*/
void StartStopTask(int task_id)
{
	PTASKDATA ptd = tasks;
	unsigned int 	task_nr;
	TaskStatus_t    xTaskDetails;

	for (task_nr = 1; ptd->func != NULL; ptd++, task_nr++)
		if (task_nr == task_id) // hebbes
		{
			vTaskGetInfo (ptd->hTask, &xTaskDetails, pdTRUE, eInvalid);
			UART_puts("\r\n");
			UART_puts(ptd->attr.name);

			// een task kan meerdere statussen hebben; het wordt simpeler
			// als je alleen de eSuspended checkt.
			if (xTaskDetails.eCurrentState == eSuspended)
			{
				vTaskResume(ptd->hTask);
				UART_puts(" resumed.");
			}
			else // bv. eBlocked, eRunning, eInvalid etc.
			{
				vTaskSuspend(ptd->hTask);
				UART_puts(" suspended.");
			}
			return;
		}
}


/**
* @brief Displays de stack-gegevens van alle taken op de UART
* @return void
*/
void CheckTaskData(int output)
{
	PTASKDATA     ptd = tasks;
	UBaseType_t   highwatermark;
	TaskStatus_t  xTaskDetails;
	unsigned int  task_nr;
	unsigned long totalalloc = 0;
	unsigned long stacksize, free, used;
	int           i;

	for (task_nr = 1; ptd->func != NULL; ptd++, task_nr++)
	{
		highwatermark = uxTaskGetStackHighWaterMark(ptd->hTask); 	// amount of free bytes
		stacksize = ptd->attr.stack_size * 4;
		free = highwatermark * 4;
		used = 100 - ((free * 100) / stacksize); 		// in percentage
		totalalloc += stacksize;

		vTaskGetInfo (ptd->hTask, &xTaskDetails, pdTRUE, eInvalid); // get task status
		if (output)
		{
			UART_puts("\r\n\t[");        UART_putint(task_nr); UART_puts("] ");
			UART_puts(ptd->attr.name);
			UART_puts("\t priority: ");  UART_putint(ptd->attr.priority);
			UART_puts("\t stacksize: "); UART_putint(stacksize); UART_puts(" bytes");
			UART_puts("\t free: ");  	 UART_putint(free);      UART_puts(" bytes");
			UART_puts("\t used: ");      UART_putint(used);      UART_puts("%");
			UART_puts("\t status: ");    UART_puts(xTaskDetails.eCurrentState == eSuspended ? "suspended": "running");
		}

		if (free < (10 * 4)) // oops, too little stackspace reserved
		{
			BUZZER_put(1000);
			Uart_debug_out = DEBUG_OUT_NONE;
			osDelay(750);
			UART_puts("\r\n==== ERROR NO STACKSPACE LEFT =====\r\n");
			UART_puts("\r\n\t[");        UART_putint(task_nr); UART_puts("] ");
			UART_puts(ptd->attr.name);
			UART_puts("\t stacksize: "); UART_putint(stacksize); UART_puts(" bytes");
			UART_puts("\t free: ");  	 UART_putint(free);      UART_puts(" bytes");
			UART_puts("\t used: ");      UART_putint(used);      UART_puts("%\r\n");

			for (i=0; i<10; i++)  // extra visual output
			{
				BUZZER_put(1000);
				LED_put(0);       // set leds
				osDelay(750);
				LED_put(255);     // set leds
			}
			error_HaltOS("Error Task Stackspace");
		}
	}

	if (output)
	{
		UART_puts("\r\n\tTotal heap: ");         UART_putint(configTOTAL_HEAP_SIZE);
		UART_puts("    Allocated task stack: "); UART_putint(totalalloc);
		UART_puts("    Free heap space: ");      UART_putint(configTOTAL_HEAP_SIZE - totalalloc);
		UART_puts("\r\n");
	}
}


