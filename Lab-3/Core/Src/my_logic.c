/*
 * my_logic.c
 *
 *  Created on: Oct 29, 2023
 *      Author: kastr
 */
#include <stdio.h>

#include "my_logic.h"
#include "my_string.h"
#include "my_sound.h"
#include "my_uart.h"
#include "gpio.h"
#include "kb.h"

uint8_t Row[4] = { ROW1, ROW2, ROW3, ROW4 }, key, old_key, flag = 0;
uint8_t mode = 1; // 0 - test, 1 - prod

uint8_t readKB() {
	uint8_t key = 0x0D;
	for (int i = 0; i < 4; i++) {
		key = Get_Key(Row[i]);
		if (key != 0x0D)
			return key;
	}
	return key;
}

void parse_mode() {
	check_mode_status();
	if (mode){
		parse_symbol_if_available();
	} else {
		parse_symbol_as_test();
	}
}


void parse_symbol_if_available() {
	key = readKB();
	if (key != old_key) {
		if (key > 0 && key < 8) {
			parse_symbol(key + '0');
		} else if (key == 8) {
			parse_symbol('-');
		} else if (key == 9) {
			parse_symbol('+');
		} else if (key == 0x0A) {
			parse_symbol('a');
		} else if (key == 0x0B) {
			parse_symbol('\r');
		} else if (key == 0x0C) {
			parse_symbol('A');
		}
		old_key = key;
	}

}
void parse_symbol_as_test() {
	key = readKB();
	if (key != old_key) {
		old_key = key;
		if (key != 0xD) {
			char answer[100];
			sprintf(answer, "You press key: %d", key);
			append_to_sending_buffer_by_uart(answer);
		}
	}
}


//public functions start
void parse_symbol(char symbol) {
	char answer[100];
	if (symbol >= '1' && symbol <= '7') {
		uint32_t note_index = symbol - '1';
		play(note_index);
	} else {
		switch (symbol) {
		case '+':
			octave_increase_if_available();
			sprintf(answer, "New octave_index is: %d",
					get_current_octave_number());
			append_to_sending_buffer_by_uart(answer);
			break;
		case '-':
			octave_decrease_if_available();
			sprintf(answer, "New octave_index is: %d",
					get_current_octave_number());
			append_to_sending_buffer_by_uart(answer);
			break;
		case 'a':
			duration_decrease_if_available();
			sprintf(answer, "New duration is: %d ms", get_current_duration());
			append_to_sending_buffer_by_uart(answer);
			break;
		case 'A':
			duration_increase_if_available();
			sprintf(answer, "New duration is: %d ms", get_current_duration());
			append_to_sending_buffer_by_uart(answer);
			break;
		case '\r':
			start_all_playing();
			break;
		default:
			sprintf(answer, "Incorrect symbol %u", symbol);
			append_to_sending_buffer_by_uart(answer);
			break;
		}
	}
}
uint8_t get_BTN() {
	return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15);
}

void check_mode_status() {
	if (get_BTN() == 0) {
		mode = 1 - mode;
		HAL_Delay(500);
		char answer[100];
		if (mode)
			sprintf(answer, "Your new mode is PROD");
		else
			sprintf(answer, "Your new mode is TEST");
		append_to_sending_buffer_by_uart(answer);
	}
}



