#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "cam.h"
#include <stdlib.h>

// Configuration
#define PWM_R 19
#define DIR_R 18
#define PWM_L 17
#define DIR_L 16
#define WRAP 99
#define BASE_SPEED 80
#define MIN_SPEED 30
#define MAX_POSITION 40
#define DEADBAND 5

typedef struct {
    uint pwm_pin;
    uint dir_pin;
    int duty;
} Motor;

Motor left = {PWM_L, DIR_L, BASE_SPEED};
Motor right = {PWM_R, DIR_R, BASE_SPEED};

void update_motor(Motor* motor) {
    uint level = WRAP - ((motor->duty * WRAP) / 100);
    gpio_put(motor->dir_pin, 1); // Forward direction
    pwm_set_gpio_level(motor->pwm_pin, level);
}

void pin_init() {
    gpio_set_function(left.pwm_pin, GPIO_FUNC_PWM);
    gpio_init(left.dir_pin);
    gpio_set_dir(left.dir_pin, GPIO_OUT);
    
    gpio_set_function(right.pwm_pin, GPIO_FUNC_PWM);
    gpio_init(right.dir_pin);
    gpio_set_dir(right.dir_pin, GPIO_OUT);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 250.0f);
    pwm_config_set_wrap(&config, WRAP);
    
    pwm_init(pwm_gpio_to_slice_num(left.pwm_pin), &config, true);
    pwm_init(pwm_gpio_to_slice_num(right.pwm_pin), &config, true);
}

void calculate_duties(int position, int* left_duty, int* right_duty) {
    if(abs(position) < DEADBAND) position = 0;
    
    if(position == 0) {
        *left_duty = BASE_SPEED;
        *right_duty = BASE_SPEED;
    }
    else if(position > 0) {
        float factor = (float)position/MAX_POSITION;
        *left_duty = BASE_SPEED;
        *right_duty = BASE_SPEED - (BASE_SPEED - MIN_SPEED) * factor;
    }
    else {
        float factor = (float)-position/MAX_POSITION;
        *right_duty = BASE_SPEED;
        *left_duty = BASE_SPEED - (BASE_SPEED - MIN_SPEED) * factor;
    }
    
    *left_duty = *left_duty < 0 ? 0 : (*left_duty > 100 ? 100 : *left_duty);
    *right_duty = *right_duty < 0 ? 0 : (*right_duty > 100 ? 100 : *right_duty);
}

int main() {
    stdio_init_all();
    pin_init();
    init_camera_pins();

    while (!stdio_usb_connected()) {}
    printf("Line Following Robot Ready\n");

    int prev_position = 0;
    int prev_left = 0;
    int prev_right = 0;

    while(1) {
        // Capture and process image
        setSaveImage(1);
        while(getSaveImage() == 1){}
        convertImage();
        int com = findLine(IMAGESIZEY/2);
        int position = com - (IMAGESIZEX/2);

        // Calculate new duties
        calculate_duties(position, &left.duty, &right.duty);
        
        // Update only if changed
        if(position != prev_position || left.duty != prev_left || right.duty != prev_right) {
            update_motor(&left);
            update_motor(&right);
            
            // Print with timestamp and newline
            absolute_time_t t = get_absolute_time();
            printf("[%8lld] Pos: %4d | L: %3d%% | R: %3d%%\n", 
                  to_us_since_boot(t), position, left.duty, right.duty);
            
            prev_position = position;
            prev_left = left.duty;
            prev_right = right.duty;
        }

        
        sleep_ms(10);
    }
}
