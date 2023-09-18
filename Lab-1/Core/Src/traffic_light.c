//
// Created by Maksim Kastricyn on 18.09.2023.
//
#include "traffic_light.h"

static duration_params default_duration_params = (duration_params) {
        .green_duration = 2 * SECOND,
        .yellow_duration = 1 * SECOND,
        .red_duration = 4 * 2 * SECOND,
        .green_blinking_duration = 1500 * MILLISECOND,

        .green_blinking_on_duration = 500 * MILLISECOND,
        .green_blinking_off_duration = 250 * MILLISECOND,
};


// gpio drivers
void turn_red_light_on() {
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
}

void turn_yellow_light_on() {
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
}

void turn_red_and_yellow_lights_off() {
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
}

void turn_green_light_on() {
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
}

void turn_green_light_off() {
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
}

uint32_t get_passed_time(uint32_t from) {
    return HAL_GetTick() - from;
}

_Bool get_current_button_state() {
    return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15);
}



// private implementation
static void set_next_mode(state *s) {
    s->current_mode = (s->current_mode + 1) % mode_length;
    s->last_update_mode = now();
}

static void set_next_blinking_mode(state *s) {
    s->current_blinking_mode = !s->current_blinking_mode;
    s->last_update_blinking_mode = now();
}

static void set_current_mode(state *s) {
    switch (s->current_mode) {
        case RED:
            turn_green_light_off();
            turn_red_light_on();
            break;
        case GREEN:
            turn_red_and_yellow_lights_off();
            turn_green_light_on();
            break;
        case GREEN_BLINKING:
            switch (s->current_blinking_mode) {
                OFF:
                    turn_red_and_yellow_lights_off();
                    turn_green_light_off();
                ON:
                    turn_red_and_yellow_lights_off();
                    turn_green_light_on();
                default:
                    //  todo err
                	break;
            }
            break;
        case YELLOW:
            turn_green_light_off();
            turn_yellow_light_on();
            break;
        default:
            //  todo err
        	break;
    }

}


// high-level function
void check_current_mode(state *s, duration_params *dp) {
    if (dp == NULL) {
        dp = &default_duration_params;
    }
    switch (s->current_mode) {
        case RED:
            if (get_passed_time(s->last_update_mode) > dp->red_duration)
                set_next_mode(s);
            break;
        case GREEN:
            if (get_passed_time(s->last_update_mode) > dp->green_duration)
                set_next_mode(s);
            // return to initial conditions
            dp->red_duration = 4 * dp->green_duration;
            break;
        case GREEN_BLINKING:
            if (get_passed_time(s->last_update_mode) > dp->green_blinking_duration)
                set_next_mode(s);
            else
                switch (s->current_blinking_mode) {
                    ON:
                        if (get_passed_time(s->last_update_blinking_mode) > dp->green_blinking_on_duration)
                            set_next_blinking_mode(s);
                    OFF:
                        if (get_passed_time(s->last_update_blinking_mode) > dp->green_blinking_off_duration)
                            set_next_blinking_mode(s);
                    default:
                        //  todo err
                    	break;
                }
            break;
        case YELLOW:
            if (get_passed_time(s->last_update_mode) > dp->yellow_duration)
                set_next_mode(s);
            break;
        default:
            //  todo err
        	break;
    }
    set_current_mode(s);
}

void check_button(state *s, duration_params *dp) {
    if (get_current_button_state())
        switch (s->current_mode) {
            case GREEN_BLINKING:
            case YELLOW:
            case RED:
                dp->red_duration = dp->green_duration;
                break;
            default:
                //todo err
            	break;
        }
}

