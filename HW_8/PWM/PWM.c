
#include "pico/stdlib.h"

#define MOTOR_PIN 17

int main() {
    gpio_set_function(MOTOR_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(MOTOR_PIN);

    float div = 64.0;          
    uint16_t wrap = 46875; //150 /(div + freq)
    pwm_set_clkdiv(slice_num, div);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true);

    float pulse_min = 0.5;  
    float pulse_max = 2.5;  

    while (1) {
        // Sweep from 0° to 180°
        for (int angle = 0; angle <= 180; angle++) {
            float pulse_width = pulse_min + (angle / 180.0) * (pulse_max - pulse_min);
            uint16_t duty = (pulse_width / 20) * wrap;
            pwm_set_gpio_level(MOTOR_PIN, duty);
            sleep_ms(20);
        }
        // Sweep back to 0°
        for (int angle = 180; angle >= 0; angle--) {
            float pulse_width = pulse_min + (angle / 180.0) * (pulse_max - pulse_min);
            uint16_t duty = (pulse_width / 20) * wrap;
            pwm_set_gpio_level(MOTOR_PIN, duty);
            sleep_ms(20);
        }
    }
}
