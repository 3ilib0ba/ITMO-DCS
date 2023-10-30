#include <stdbool.h>
#include <string.h>

#include "usart.h"
#include "my_uart.h"

//#include "uart_our_driver.h"

bool is_writing_now = false;

char read_buffer[100];
char write_buffer[100];

char *cur_process_char = read_buffer;
char *cur_read_char = read_buffer;
char *transmit_from_pointer = write_buffer;
char *write_to_pointer = write_buffer;

//public functions start
char* read_char_by_uart() {
	char* ans = NULL;
	if (cur_process_char != cur_read_char) {
		ans = cur_process_char;
		next(&cur_process_char, read_buffer);
	}
	return ans;
}

void send_message_by_uart_if_available() {
	if (!is_writing_now) {
		if (transmit_from_pointer != write_to_pointer) {
			is_writing_now = true;
			HAL_UART_Transmit_IT(&huart6, (uint8_t*) transmit_from_pointer,
					sizeof(char));
		}
	}
}

void set_default_value_for_uart() {
	HAL_UART_Receive_IT(&huart6, (uint8_t*) cur_read_char, sizeof(char));
}

void append_to_sending_buffer_by_uart(char *str) {
//	char *str_with_newline = concat("\r\n", str);
	char *str_with_newline = concat(str, "\r\n");
	int size = sizeof(char) * strlen(str_with_newline);
	for (size_t i = 0; str_with_newline[i] != '\0'; i++) {
		write_char_to_buff(str_with_newline[i]);
	}
}

//Callback of interrupt by uart

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == huart6.Instance) {
		next(&cur_read_char, read_buffer);
		HAL_UART_Receive_IT(&huart6, (uint8_t*) cur_read_char, sizeof(char));
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == huart6.Instance) {
		is_writing_now = false;
		next(&transmit_from_pointer, write_buffer);
	}
}
//public functions end

void write_char_to_buff(char c) {
	*write_to_pointer = c;
	next(&write_to_pointer, write_buffer);
}

void next(char **pointer, char *buffer) {
	if (*pointer >= buffer + 100)
		*pointer = buffer;
	else
		(*pointer)++;
}
