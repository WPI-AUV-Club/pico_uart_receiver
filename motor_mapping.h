#ifndef _MOTOR_MAP_H
#define _MOTOR_MAP_H

#include "pwm_manager.h"

//Motor GPIO Pin Mapping
#define MOTOR_0 2
#define MOTOR_1 3
#define MOTOR_2 4
#define MOTOR_3 5
#define MOTOR_4 6
#define MOTOR_5 7
#define MOTOR_6 8
#define MOTOR_7 9


/*! \brief Initialize PWM channels for each motor
* \ingroup motor_map
*/
void init_motors() {
    init_pwm_pin(MOTOR_0);
    init_pwm_pin(MOTOR_1);
    init_pwm_pin(MOTOR_2);
    init_pwm_pin(MOTOR_3);
    init_pwm_pin(MOTOR_4);
    init_pwm_pin(MOTOR_5);
    init_pwm_pin(MOTOR_6);
    init_pwm_pin(MOTOR_7);
}

/*! \brief Set all motors to full stop
* \ingroup motor_map
*
* \param pin throttle value [0 full reverse, 127 full stop, 254 full forward]
*/
void stop_all_motors() {
    set_pwm_pin(MOTOR_0, FULL_STOP_THROTTLE);
    set_pwm_pin(MOTOR_1, FULL_STOP_THROTTLE);
    set_pwm_pin(MOTOR_2, FULL_STOP_THROTTLE);
    set_pwm_pin(MOTOR_3, FULL_STOP_THROTTLE);
    set_pwm_pin(MOTOR_4, FULL_STOP_THROTTLE);
    set_pwm_pin(MOTOR_5, FULL_STOP_THROTTLE);
    set_pwm_pin(MOTOR_6, FULL_STOP_THROTTLE);
    set_pwm_pin(MOTOR_7, FULL_STOP_THROTTLE);
}

/*! \brief Given an array of throttles, set each motors speed
* \ingroup motor_map
*
* \param motor_speeds array of speeds [0 full reverse, 127 full stop, 254 full forward]
*/
void set_motor_speeds(char motor_speeds[]) {
        set_pwm_pin(MOTOR_0, motor_speeds[0]);
        set_pwm_pin(MOTOR_1, motor_speeds[1]);
        set_pwm_pin(MOTOR_2, motor_speeds[2]);
        set_pwm_pin(MOTOR_3, motor_speeds[3]);
        set_pwm_pin(MOTOR_4, motor_speeds[4]);
        set_pwm_pin(MOTOR_5, motor_speeds[5]);
        set_pwm_pin(MOTOR_6, motor_speeds[6]);
        set_pwm_pin(MOTOR_7, motor_speeds[7]);
}

#endif