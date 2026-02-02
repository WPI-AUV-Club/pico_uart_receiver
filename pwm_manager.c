#include "pwm_manager.h"
#include "hardware/pwm.h"


/*! \brief Initialize a pin for PWM output
 *  \ingroup pwm_manager
 *
 * Configure the PWM slice and channel for the given PWM at PWM_FREQ 
 *
 * \param pin gpio pin
 */
void init_pwm_pin(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_set_enabled(slice_num, true);
}

/*! \brief Set a PWM output to control an ESC
 *  \ingroup pwm_manager
 *
 * Translate the given throttle value to a pwm duty cycle appropriate to control the ESC 
 *
 * \param pin gpio pin
 * \param throttle throttle value [0 full reverse, 127 full stop, 254 full forward]
 */
void set_pwm_pin(uint pin, char throttle) {
    int duty_cycle;
    if (throttle == 127) {
        duty_cycle = get_duty_cycle(PWM_FREQ, FULL_STOP_MS);
    } else {
        float pulse_len_ms = naive_lerp(FULL_REVERSE_MS, FULL_FORWARD_MS, throttle/254.0f);
        duty_cycle = get_duty_cycle(PWM_FREQ, pulse_len_ms);
    }

    uint slice_num = pwm_gpio_to_slice_num(pin);
    uint chan = pwm_gpio_to_channel(pin);
    pwm_set_freq_duty(slice_num, chan, PWM_FREQ, duty_cycle);
}


static int get_duty_cycle(uint freq, float pulse_len_ms) {
    float total_pulse_ms = 1.0/freq*1000;
    return (pulse_len_ms/total_pulse_ms)*100;
}


static uint32_t pwm_set_freq_duty(uint slice_num, uint chan, uint32_t freq, int duty_cycle) {
    uint32_t clock = 125000000;
    uint32_t divider16 = clock / freq / 4096 + (clock % (freq * 4096) != 0);
    if (divider16 / 16 == 0) divider16 = 16;

    uint32_t wrap = clock * 16 / divider16 / freq - 1;

    pwm_set_clkdiv_int_frac4(slice_num, divider16/16, divider16 & 0xF);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_chan_level(slice_num, chan, wrap * duty_cycle / 100);

    return wrap;
}


static float naive_lerp(float a, float b, float t) {
    return a + t*(b - a);
}