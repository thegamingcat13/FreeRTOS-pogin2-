/**
 * @file ultrasonic.c
 * @brief Task/ voor het:
 *
 * \- Aanzetten van de sensor
 * \- Uitlezen van sensor
 * \- Beslissingen maken aan de hand van data sensor
 *
 * @date 30/10/2025
 */

#include "admin.h"
#include "main.h"
#include "cmsis_os.h"
#include "gps.h"
#include "ultrasonic.h"
#include "usb_host.h"
#include <stdlib.h>
#include <stdbool.h>

/**
 * @brief Provides a blocking microsecond delay using timer 2
 * @param us: Number of microseconds to delay
 */

bool sr04_debug = false;
EventGroupHandle_t xEchoEventGroup; // Event group for signaling pulse completion

volatile uint32_t rising_edge_time_us = 0;
volatile uint32_t falling_edge_time_us = 0;
volatile uint8_t echo_state = 0;
volatile uint32_t pulse_duration_us = 0;

float distance_cm;

/**
 * @brief Functie voor het creeren van de timer, nodig voor het corect werken van de sensor.
 * @param
 */
void delay_us(int us)
{
	__HAL_TIM_SET_COUNTER(&htim2, 0); 				// Reset timer 2 counter
	HAL_TIM_Base_Start(&htim2);						// Start timer 2
	while (__HAL_TIM_GET_COUNTER(&htim2) < us);		// wait until specified time
	HAL_TIM_Base_Stop(&htim2);						// Stop timer 2
}


/**
 * @brief Functie voor het aansturen en het uitlezen van de sensor. Daarnaast berekent hij de afstand, en maakt hij een keuze op basis van deze afstand
 * @param *argument Niet gebruikt
 * @return void
 */
void SR04_Task (void *argument)
{
	int current_duration_us;

	HAL_TIM_Base_Start(&htim5);

	while (TRUE)
	{
		// Reset variables for the new measurement
		rising_edge_time_us = 0;
		falling_edge_time_us = 0;
		echo_state = 0;
		pulse_duration_us = 0;

		// Clear the eventgroup for the new measurement
		xEventGroupClearBits(xEchoEventGroup, ULTRASONIC_PULSE_COMPLETE_BIT);

		// Clear any pending interrupts for the Echo pin
		__HAL_GPIO_EXTI_CLEAR_IT(Echo_Pin);
		HAL_NVIC_ClearPendingIRQ(EXTI15_10_IRQn);

		// Trigger the sensor
		HAL_GPIO_WritePin(Trigger_GPIO_Port, Trigger_Pin, RESET);
		delay_us(2);
		HAL_GPIO_WritePin(Trigger_GPIO_Port, Trigger_Pin, SET);
		delay_us(10);
		HAL_GPIO_WritePin(Trigger_GPIO_Port, Trigger_Pin, RESET);

		// Wait for the pulse to complete
		EventBits_t uxBits = xEventGroupWaitBits(
				xEchoEventGroup,					// Eventgroup to wait for
				ULTRASONIC_PULSE_COMPLETE_BIT,		// Bit to wait for
				pdTRUE,								// Clear the bit on exit
				pdFALSE,							// Don't wait for all bits
				pdMS_TO_TICKS(100)					// Timeout after 100ms
				);

		if ((uxBits & ULTRASONIC_PULSE_COMPLETE_BIT) == ULTRASONIC_PULSE_COMPLETE_BIT)
		{
			current_duration_us = pulse_duration_us;
			distance_cm = (current_duration_us * 0.0343f) / 2.0f;

			if (distance_cm > 40.0f || distance_cm < 2.0f)
				distance_cm = -1.0f;

			if (distance_cm < 20 && distance_cm > 0)
			{
				drive_backward();
				osDelay(1000);
				turn_left();
				osDelay(1000);
				drive_forward();
				osDelay(1000);
				turn_right();
				osDelay(1000);
				stop();
			}
			if (sr04_debug)
				UART_printf(100, "\n\n\rDistance: %.2f cm", distance_cm);
		} else
			if (sr04_debug)
				UART_printf(100, "\n\n\r Error: No distance measured");

		vTaskDelay(pdMS_TO_TICKS(500));
	}
}
