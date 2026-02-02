#ifndef _PWM_MANAGER_H
#define _PWM_MANAGER_H

#include "pico/stdlib.h"

#define FULL_FORWARD_MS 1.9
#define FULL_REVERSE_MS 1.1
#define FULL_STOP_MS 1.5
#define FULL_STOP_THROTTLE 127
#define PWM_FREQ 400

//Motor GPIO Pin Mapping
#define MOTOR_0 2
#define MOTOR_1 3
#define MOTOR_2 4
#define MOTOR_3 5
#define MOTOR_4 6
#define MOTOR_5 7
#define MOTOR_6 8
#define MOTOR_7 9

// GPIO         0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 
// GPIO        16 17 18 19 20 21 22 23 24 25 26 27 28 29
// PWM Channel 0A 0B 1A 1B 2A 2B 3A 3B 4A 4B 5A 5B 6A 6B 7A 7B

// Two channels of the same slice run at the same frequency, but can have a different duty rate. 
// 3A -> Slice 3, channel A

void init_pwm_pin(uint pin);
void set_pwm_pin(uint pin, char throttle);
void set_all_pins(char throttle);

static uint32_t pwm_set_freq_duty(uint slice_num, uint chan, uint32_t freq, float duty_cycle);
static float get_duty_cycle(uint freq, float pulse_len_ms);
static float naive_lerp(float a, float b, float t);

#endif