/* ! Echtzeitbetriebssysteme Task 3
 * ! Serial Communication / UART / Thread / PWM
 *
 * \description A mbed-os Serial UART PWM Programm
 *
 * \author Sebastian Dichler <el16b032@technikum-wien.at> <sedi343@gmail.com>
 *
 * \version Rev.: 01, 16.04.2018 - Created cpp file
 *                02, 24.04.2018 - Wrote the whole code we need for this task
 *
 * \notes
 *
 * \commands #0:LED1:DATA$ ... DATA represents the percentage (0-100) and
 *                             controls the LED1s duty cycle
 *           #0:LED2:DATA$ ... DATA represents the percentage (0-100) and
 *                             controls the LED2s duty cylce
 *           #0:RES:NULL$ .... RES represents the reset of current Tasks (LED1
 *                             or LED2)
 */

/********************************** Includes **********************************/
#include "mbed.h"
#include "rtos.h"
#include <stdlib.h>
#include <string>
#include <cstring>
#include <iostream>

/********************************** Defines ***********************************/
#define UARTTX P1_5
#define UARTRX P1_4

#define DEBUG 1
#define BUF_SIZE 16

/********************************** Globals ***********************************/
/*
	LED1 and LED2 are defined in:
	targets/TARGET_Infineon/TARGET_XMC4XXX/TARGET_XMC4500/PinNames.h
*/
PwmOut led_1(LED1);					/* LED1 = P1.1 */
PwmOut led_2(LED2);					/* LED2 = P1.0 */

Serial device(UARTTX, UARTRX);	/* UART -> TX = P1.5, RX = P1.4 */

/* Create Threads */
Thread *thread_led1;									/* Thread for LED1 */
Thread *thread_led2;									/* Thread for LED2 */
Thread *thread_com;										/* Thread for communication */

float value_led1;
float value_led2;

/********************************* Functions **********************************/
void com_led_1(void)
{
	while (1)
	{
		led_1.write(value_led1);
		Thread::wait(10);
	}
}

void com_led_2(void)
{
	while (1)
	{
		led_2.write(value_led2);
		Thread::wait(10);
	}
}

void com_communication(void)
{
	osStatus status;
	char receiver_char;
	char receiver_buffer[BUF_SIZE];
	uint32_t receiver_counter = 0;
	uint32_t i = 0;
	
	char *ptr;
	char *savepointer;
	char *token;
	
	char *number_pointer;
	char *command_pointer;
	char *data_pointer;
	
	char number_buffer[BUF_SIZE];
	char command_buffer[BUF_SIZE];
	char data_buffer[BUF_SIZE];
	
	using namespace std;
	
	/*
		Configuration of the UART,
		Change Baudrate to 9600
	*/
	device.baud(9600);
	
	device.printf("Threaded UART PWM Task\n");
	device.printf("by Sebastian Dichler | <el16b032@technikum-wien.at>\n");
	device.printf("Task 1 - Communication\n");
	device.printf("Task 2 - Control of LED1\n");
	device.printf("Task 3 - Control of LED2\n\n");
	
	while (1)
	{
		/* Initialize the Buffer with 0 */
		memset(receiver_buffer, 0, BUF_SIZE*sizeof(char));
		
		/* Reset Buffers and other variables */
		memset(number_buffer, 0, BUF_SIZE*sizeof(char));
		memset(command_buffer, 0, BUF_SIZE*sizeof(char));
		memset(data_buffer, 0, BUF_SIZE*sizeof(char));
		receiver_counter = 0;
		
		/* If the UART is readable */
		if (device.readable())
		{
			/* Remove Characters until received char equals # */
			receiver_char = device.getc();
			if (receiver_char == '#')
			{
				/*
					Read characters and write them to buffer
					until char is $ or BUF_SIZE is reached
				*/
				while (receiver_char != '$' && receiver_counter != BUF_SIZE)
				{
					if (device.readable())
					{
						/* Put every received character into the buffer */
						receiver_char = device.getc();
						receiver_buffer[receiver_counter] = receiver_char;
						
						receiver_counter++;
					}
				}
				
				receiver_buffer[BUF_SIZE] = '\0';
				
				/* Send ACK */
				device.putc(0x06);
				
#if DEBUG
				device.printf("%s\n", receiver_buffer);
#endif
				
				i = 0;
				
				/* Split input buffer into commands */
				token = strtok_r(receiver_buffer, ":", &savepointer);
				while (token != NULL)
				{
					if (i == 0)
					{
						number_pointer = token;
					}
					
					if (i == 1)
					{
						command_pointer = token;
					}
					
					if (i == 2)
					{
						data_pointer = token;
					}
					
					token = strtok_r(NULL, ":", &savepointer);
					i++;
				}
				
				/* Copy splitted command into buffers */
				memcpy(number_buffer, (char *)number_pointer, BUF_SIZE*sizeof(char));
				memcpy(command_buffer, (char *)command_pointer, BUF_SIZE*sizeof(char));
				memcpy(data_buffer, (char *)data_pointer, BUF_SIZE*sizeof(char));
				data_buffer[strlen(data_buffer)-1] = '\0';
				
#if DEBUG
				device.printf("Number:  %s\n", number_buffer);
				device.printf("Command: %s\n", command_buffer);
				device.printf("Data:    %s\n", data_buffer);
#endif
				
				/* LED1 Command */
				if (strncmp(command_buffer, "LED1", BUF_SIZE*sizeof(char)) == 0)
				{
					value_led1 = strtof(data_buffer, &ptr) / 100;
					
#if DEBUG
					device.printf("LED1 Value: %.2f%%\n", value_led1 * 100);
#endif
					
					if (thread_led1 == NULL)
					{
						thread_led1 = new Thread();
					}
					
					status = thread_led1->start(com_led_1);
					if (status != osOK)
					{
						error("ERROR: Thread LED1: Failed!");
					}
				} // strncmp LED1
				
				/* LED2 Command */
				else if (strncmp(command_buffer, "LED2", BUF_SIZE*sizeof(char)) == 0)
				{
					value_led2 = strtof(data_buffer, &ptr) / 100;
					
#if DEBUG
					device.printf("LED2 Value: %.2f%%\n", value_led2 * 100);
#endif
					
					if (thread_led2 == NULL)
					{
						thread_led2 = new Thread();
					}
					
					status = thread_led2->start(com_led_2);
					if (status != osOK)
					{
						error("ERROR: Thread LED2: Failed!");
					}
				} // strncmp LED2
				
				/* RESET Command */
				else if (strncmp(command_buffer, "RES", BUF_SIZE*sizeof(char)) == 0)
				{
					if (thread_led1 != NULL)
					{
						thread_led1->terminate();
						delete thread_led1;
						thread_led1 = NULL;
						led_1.write(0.0);
						device.printf("NOT DONE Task 1\n");
					}
					
					if (thread_led2 != NULL)
					{
						thread_led2->terminate();
						delete thread_led2;
						thread_led2 = NULL;
						led_2.write(0.0);
						device.printf("NOT DONE Task 2\n");
					}
				} // stncmp RES
				
				device.printf("\n");
				Thread::wait(10);
			} // receiver_char == #
		} // device.readable()
	} // while (1)
}

/******************************* Main Function ********************************/

int main(void)
{
	osStatus status;
	
	led_1.period_ms(10);
	led_2.period_ms(10);
	
	led_1.write(0);
	led_2.write(0);

		thread_com = new Thread();
	
/* ---- Start Thread 1 ---- */
	status = thread_com->start(com_communication);
	if (status != osOK)
	{
		error("ERROR: Thread 1: Failed!");
	}
}
