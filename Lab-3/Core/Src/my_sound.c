/*
 * my_sound.c
 *
 *  Created on: Oct 29, 2023
 *      Author: kastr
 */
#include "tim.h"

#include "my_uart.h"
#include "my_sound.h"

const uint32_t OCTAVE_SIZE = 7;
uint32_t freqs[] = { 0, 0, 20610, 21820, 24500, 27500, 30870, 32700, 36950,
		41210, 43650, 49000, 55000, 61740, 65410, 73910, 82410, 87310, 98000,
		110000, 123480, 130820, 147830, 164810, 174620, 196000, 220000, 110000,
		261630, 293670, 329630, 349230, 392000, 440000, 493880, 523260, 587340,
		659260, 698460, 784000, 880000, 987760, 1046520, 1174680, 1318500,
		1396900, 1568000, 1720000, 1975500, 2093000, 2349200, 2637000, 2739800,
		3136000, 3440000, 3951000, 4186000, 4698400, 5274000, 0, 0, 0, 0 };
uint32_t note_index = 0;
uint32_t octave_index = 4;
uint32_t duration = 1000;
uint8_t is_all_playing = 0;
char *note_name[] = { "До", "Ре", "Ми", "Фа", "Соль", "Ля", "Си" };

void restart_timer() {
	TIM6->CNT = 0;
}

void mute() {
	TIM1->CCR1 = 0;
}

void unmute() {
	TIM1->CCR1 = TIM1->ARR / 2;
}

void set_frequency(uint32_t freq_millis) {
	TIM1->PSC = (1125000) / ((freq_millis / 1000) * (TIM1->ARR + 1)) -1;
}

uint32_t get_frequency_by_index(uint32_t index) {
	return freqs[index + (octave_index * OCTAVE_SIZE)];
}

//public functions start

void set_default_value_for_sound() {
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_Base_Start_IT(&htim6);
	TIM6->ARR = duration;
}

uint32_t get_current_octave_number() {
	return octave_index + 1;
}
uint32_t get_current_duration() {
	return duration;
}
char* get_note_name(uint32_t index) {
	return note_name[index];
}

void play(uint32_t note_index) {
	int freq = get_frequency_by_index(note_index);
	if (freq > 0) {
		set_frequency(freq);
		restart_timer();
		unmute();
		char answer[100];
		sprintf(answer, "Note: %s, octave: %d", get_note_name(note_index),
				get_current_octave_number());
		append_to_sending_buffer_by_uart(answer);
	} else {
		if (is_all_playing) {
			mute();
			is_all_playing = 0;
		} else {
			char answer[100];
			sprintf(answer, "There is no note: %s in octave: %d!",
					note_name[note_index], octave_index + 1);
			append_to_sending_buffer_by_uart(answer);
		}
	}

}

void start_all_playing() {
	is_all_playing = 1;
	if (octave_index > 0)
		note_index = 0;
	else
		note_index = 2;
	play(note_index);
}

void octave_decrease_if_available() {
	if (octave_index > 0)
		octave_index--;
}

void octave_increase_if_available() {
	if (octave_index < 8)
		octave_index++;
}

void duration_decrease_if_available() {
	if (duration > 100) {
		duration -= 100;
		TIM6->ARR = duration;
		restart_timer();
	}
}

void duration_increase_if_available() {
	if (duration < 5000) {
		duration += 100;
		TIM6->ARR = duration;
		restart_timer();
	}
}
//public functions start

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM6) {
		mute();
		if (is_all_playing){
			note_index++;
			if (note_index < OCTAVE_SIZE)
				play(note_index);
			else
				is_all_playing = 0;
		}
	}
}

