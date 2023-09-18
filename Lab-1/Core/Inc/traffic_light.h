//
// Created by Maksim Kastricyn on 18.09.2023.
//

#ifndef ITMO_DCS_TRAFFIC_LIGHT_H
#define ITMO_DCS_TRAFFIC_LIGHT_H

#include <stdint.h>

#include "stm32f4xx_hal.h"

extern const uint32_t MILLISECOND;
extern const uint32_t SECOND;

// state description

typedef struct {
    uint32_t green_duration;
    uint32_t yellow_duration;
    uint32_t red_duration;
    uint32_t green_blinking_duration;

    uint32_t green_blinking_on_duration;
    uint32_t green_blinking_off_duration;
} duration_params;

typedef struct {
    mode current_mode;
    uint32_t last_update_mode;
    blinking_mode current_blinking_mode;
    uint32_t last_update_blinking_mode;
} state;
typedef enum {
    OFF = 0,
    ON,
} blinking_mode;
typedef enum {
    RED = 0,
    GREEN,
    GREEN_BLINKING,
    YELLOW,
    Length
} mode;

//  gpio drivers

void turn_red_light_on();

void turn_yellow_light_on();

void turn_red_and_yellow_lights_off();

void turn_green_light_on();

void turn_green_light_off();

#define now() HAL_GetTick()

uint32_t get_passed_time(uint32_t);

_Bool get_current_button_state(){};


//high-level function

void check_current_mode(state *s, duration_params *dp);


void check_button();


#endif //ITMO_DCS_TRAFFIC_LIGHT_H
