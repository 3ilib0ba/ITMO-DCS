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

//public functions start

void parse_symbol_if_available() {
	char* symb = read_char_by_uart();
	if (symb == NULL)
		return;
	parse_symbol(*symb);
}

//public functions start
void parse_symbol(char symbol){
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
