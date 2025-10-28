#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "admin.h"

#define ULTRASONIC_PULSE_COMPLETE_BIT (1UL << 0UL) // Bit 0 to signal pulse completion with eventgroup

extern TIM_HandleTypeDef htim2; // used for delay_us function
extern TIM_HandleTypeDef htim5; // used for pulse measurement

volatile uint32_t rising_edge_time_us = 0;
volatile uint32_t falling_edge_time_us = 0;
volatile uint8_t echo_state = 0;
volatile uint32_t pulse_duration_us = 0;

EventGroupHandle_t xEchoEventGroup; // Event group for signaling pulse completion


/**
 * @brief Provides a blocking microsecond delay using timer 2
 * @param us: Number of microseconds to delay
 * @retval None
 */

void delay_us(uint32_t us)
{
	__HAL_TIM_SET_COUNTER(&htim2, 0); 				// Reset timer 2 counter
	HAL_TIM_Base_start(&htim2);						// Start timer 2
	while (__HAL_TIM_GET_COUNTER(&htim2) < us);		// wait until specified time
	HAL_TIM_Base_Stop(&htim2);						// Stop timer 2
}

