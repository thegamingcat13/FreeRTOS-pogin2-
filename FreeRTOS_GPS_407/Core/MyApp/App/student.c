/**
* @file student.c
* @brief Hier kunnen studenten hun eigen tasks aanmaken
*
* @author MSC

* @date 5/5/2022
*/
#include <admin.h>
#include "main.h"
#include "cmsis_os.h"


/**
* @brief Oefentask voor studenten
* @param argument, kan evt vanuit tasks gebruikt worden
* @return void
*/


//extern TIM_HandleTypeDef htim8;  // zeg tegen de compiler dat htim8 elders bestaat
//
//
//void setMotorSpeed(uint16_t duty)
//{
//    if (duty > htim8.Init.Period)
//        duty = htim8.Init.Period;  // beveiliging tegen te hoge waarde
//
//    __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, duty);
//
//}

void Student_task1 (void *argument)
{
	UART_puts((char *)__func__); UART_puts(" started\r\n");
	char buf[80];
	unsigned char i = 0;

	while(TRUE)
	{
//		setMotorSpeed(4200);  // 50% duty cycle (bij Period = 4200)
		// Example C code (HAL library)
		// Start the PWM channel
//		if (HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1) != HAL_OK)
//		{
//		  Error_Handler();
//		}

		if (Uart_debug_out & STUDENT_DEBUG_OUT)
		{
	       	sprintf(buf, "\r\n%s: %u", __func__, i++);
			UART_puts(buf);
    	}
	}
}
