#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "admin.h"
#include <stdbool.h>

#define ULTRASONIC_PULSE_COMPLETE_BIT (1UL << 0UL) // Bit 0 to signal pulse completion with eventgroup

extern TIM_HandleTypeDef htim2; // used for delay_us function
extern TIM_HandleTypeDef htim5; // used for pulse measurement

extern volatile uint32_t rising_edge_time_us;
extern volatile uint32_t falling_edge_time_us;
extern volatile uint8_t echo_state;
extern volatile uint32_t pulse_duration_us;

extern bool sr04_debug;

extern EventGroupHandle_t xEchoEventGroup; // Event group for signaling pulse completion

#endif // ULTRASONIC_H
