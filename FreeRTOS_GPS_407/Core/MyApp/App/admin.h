/**
* @file admin.h
* @brief Bevat de defines & externals voor deze applicatie
* @attention
* <h3>&copy; Copyright (c) 2021 (HU) Michiel Scager.</h3>
* @author MSC
*
* @date 5/5/2022
*/
#ifndef ADMIN_H
#define ADMIN_H

#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"
#include "task.h" // taskYield
#include "semphr.h"
#include "message_buffer.h"
#include "cmsis_os.h"
#include <stdbool.h>
#include "stdio.h"
#include "string.h" // strtok, strcpy
#include "stdlib.h" // atoi
#include "ctype.h"  // toupper

#define TRUE  	   1
#define FALSE      0

/// set queue op 10 elementen
#define QSIZE_UART 10

/// motor speeds defines
#define SLOW 300
#define MEDIUM 500
#define FAST 1500
#define VERY_FAST 600
#define STANDSTILL 0

// motor direction definies
#define FORWARD 1
#define BACKWARD -1
#define RIGHT 2
#define LEFT -2
#define STOP 3

/// set software timer 1000 msecs
#define TIMER1_DELAY 1000

#define GPS_MAXLEN 79+4 /// $+CR+LF+'\0'
/** The carriage return [CR] and the line feed [LF] combination terminate the sentence.
* NMEA 0 83 sentences vary in length, but each sentence is limited to 79 characters
* or less. This length limit excludes the $ and the [CR][LF] characters.
* The data field block, including delimiters is limited to 74 characters or less.
*/

#define STRC_AMOUNT 20 // amount of waypoints that will be created

#define LAT_PREC 0.000045 // level of precision for gps parser
#define LON_PREC 0.000074 // level of precision for gps parser
#define METER_PER_DEG_LAT 111132.0f
#define METER_PER_DEG_LON 65430.0f

#define MAX_HEADING_DIFFERENCE_DEG 5.0f // level of precision for the heading were going to
#define ARRIVAL_RADIUS_METERS 2.0f // set arrival radius to 2 meter
#define MIN_COG_SPEED 0.5f


// dataretention.c
#define MAX_LOGS 100 // Define the maximum number of logs

// structure that contains the info that will be used for the route compute
typedef struct RouteInfo_s{
	    float latdifference;
	    float londifference;
	    float distance_m;
	} RouteInfo;

extern RouteInfo info;

// data retention log structure
typedef struct log
{
	char    latitude[10];  // 4. ddmm.mmmm (double)
	char    longitude[10]; // 6. ddmm.mmmm (double)
	char    heading_current[8];     // 9. 309.62 degrees (double)
	char	heading_desired[8];
	char    buttons_pressed[25];          // 13. A=autonomous, D,E
	char	commands[10];	   // motor commands
} Slog;

/// struct voor taak-gegevens, waaronder de argumenten om een taak aan te maken
typedef struct TaskData
{
	osThreadFunc_t       func;     /// de functie()-pointer van de taak/thread/task zelf
	void *               argument; /// evt extra variabele voor taak, bv. "hello"
	osThreadAttr_t       attr;     /// struct for OS, but we only declare a few members
	osThreadId_t         hTask;    /// taskhandle, handig: gekregen van osThreadNew()
} TASKDATA, *PTASKDATA;			   /// typedefs of struct itself and pointer to it



/// alle handles
/// handle voor UART-queue
extern QueueHandle_t 	  hUART_Queue;
/// handle voor GPS-queue
extern QueueHandle_t 	  hGPS_Queue;
/// handle voor LED-mutex
extern SemaphoreHandle_t  hLED_Sem;
///handle voor de parsed gps structure
extern SemaphoreHandle_t  hGpsDataMutex;
/// handle voor ARM-keys-event
extern EventGroupHandle_t hKEY_Event;
/// handle voor software timer
extern TimerHandle_t      hTimer1;
/// handle zodat de waypoint task extern genotified kan worden
extern TaskHandle_t		  hReachWP;



/// debug naar uart output, zie uart_keys.c
/// bitmask-toggle key voor task-debug-output
extern int Uart_debug_out;
/// stop alle test output
#define DEBUG_OUT_NONE 		0x00
/// genereer alle test output
#define DEBUG_OUT_ALL 		0xff

/// bit 1: toggles leds stuff output
#define LEDS_DEBUG_OUT   	0x01
/// bit 2: toggles armkeys stuff output
#define ARMKEYS_DEBUG_OUT 	0x02
/// bit 3: toggles uart2 stuff output
#define UART_DEBUG_OUT    	0x04
/// bit 4: toggles student stuff output
#define STUDENT_DEBUG_OUT  	0x08
/// bit 5: toggles gps uart1 output
#define GPS_DEBUG_OUT 		0x10
/// bit 6: toggles the raw incoming gps-input
#define GPS_DEBUG_RAW_OUT   0x20
/// bit 7: toggles whatever
#define RESERVED1_OUT       0x40
/// bit 8: toggles whatever
#define RESERVED2_OUT       0x80

/// Redefine pins om beter aan te geven waar het om gaat: gekleurde ledjes
/// LD4_Pin
#define LEDGREEN  GPIO_PIN_12
/// LD3_Pin
#define LEDRED    GPIO_PIN_13
/// LD5_Pin
#define LEDORANGE GPIO_PIN_14
/// LD6_Pin
#define LEDBLUE   GPIO_PIN_15

#define M1_1 	  GPIO_PIN_7

#define M1_2	  GPIO_PIN_8

#define M2_1	  GPIO_PIN_9

#define M2_2 	  GPIO_PIN_10

#define M1_ANALOG GPIO_PIN_11

#define M2_ANALOG GPIO_PIN_12

// my_app.c
extern void DisplayVersion (void);
extern void DisplayMenu    (void);
extern void error_HaltOS   (char *);

// tasks.c
extern void         CheckTaskData (int);
extern void         CreateTasks     (void);
extern osThreadId_t GetTaskhandle   (char *);
extern void         SetTaskPriority (int, int);
extern void         StartStopTask   (int);

// handles.c
extern void         CreateHandles   (void);

// LEDS.c
extern void LED_Task1 (void *);
extern void LED_Task2 (void *);
extern void LED_Task3 (void *);
extern void LED_Task4 (void *);

//route.c
extern void Waypoint ();
extern float returnWaypoints (int, int);
extern int CurrentWaypointChange (int);

// ARM_keys.c
extern void ARM_keys_IRQ (void *);
extern void ARM_keys_task(void *);

// UART_keys.c
extern void UART_keys_poll(void *);
extern void UART_keys_IRQ (void *);
extern void UART_menu     (void *);

// gps.c
extern void GPS_getNMEA (void *);
extern void ParsedGPS (void);

// studenttask.c
extern void Student_task1 (void *);

// admin.c
extern void Timer1_Handler(void);

//TactileFeedback.c
extern void ReachWPTask (void *);
extern float heading (int);
extern RouteInfo Get_Waypoint_Info (int waypoint);
extern int SkipWaypoint (void);
extern int BackWaypoint (void);
extern void ShowWaypoint (void);
extern void setMotors(int direction, uint16_t speed_l, uint16_t speed_r);

// dataretention.c
extern void logWrite(int type, void* info);
extern void printLogs(void);

// ultrasonic.c
extern void delay_us (int us);
extern void SR04_Task (void *);

// gps.c
extern bool gps_lcd_print;

#endif // ADMIM_H
