/*
 * my_sound.h
 *
 *  Created on: Oct 29, 2023
 *      Author: kastr
 */

#ifndef INC_MY_SOUND_H_
#define INC_MY_SOUND_H_

void set_default_value_for_sound();

uint32_t get_current_octave_number();
uint32_t get_current_duration();

char* get_note_name(uint32_t note_index);

void play(uint32_t note_index);
void start_all_playing();

void octave_decrease_if_available();

void octave_increase_if_available();

void duration_decrease_if_available();

void duration_increase_if_available();
#endif /* INC_MY_SOUND_H_ */
