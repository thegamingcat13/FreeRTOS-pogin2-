#include "admin.h"
#include "main.h"
#include "cmsis_os.h"
#include "gps.h"
#include <stdlib.h>

void SR04_trigger(void)
{
	HAL_GPIO_WritePin(Trigger_GPIO_Port, Trigger_Pin, SET);
	//for (int i = 0; i < 100; i++);
	//HAL_GPIO_WritePin(Trigger_GPIO_Port, Trigger_Pin, RESET);
}

void SR04_Task(void *argument)
{
	uint32_t duration;
	float distance;

	if (xSR04DistanceQueue == NULL)
		vTaskSuspend(NULL);

	while (TRUE)
	{
		SR04_trigger();

		if (xQueueReceive(xSR04DistanceQueue, &duration, pdMS_TO_TICKS(100)) == pdPASS)
		{
			distance = (float)duration * 0.01715f;
			UART_printf(100, "\n\n\r Distance: %.2f cm", distance);
		} else
			UART_printf(100, "\n\n\r measure timeout");

		vTaskDelay(pdMS_TO_TICKS(500));
	}
}
