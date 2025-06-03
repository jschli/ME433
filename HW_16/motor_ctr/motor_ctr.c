#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <math.h>
#include <stdlib.h>

// Pin definitions
#define PWM_R 19
#define DIR_R 18
#define PWM_L 17
#define DIR_L 16

// PWM settings
#define WRAP 99
#define DIV 150.0
#define DUTY_MAX 100
#define DUTY_MIN -100

// Motor control structure
typedef struct {
    uint pwm_pin;
    uint dir_pin;
    int duty;
} Motor;

Motor left = {PWM_L, DIR_L, 0};
Motor right = {PWM_R, DIR_R, 0};

void update_motor(Motor* motor) {
    
    uint level = 0; 
    gpio_put(motor->dir_pin, 0);
    int absolute_duty = abs(motor->duty);

    if (motor->duty > 0){
    level = WRAP - ((absolute_duty * WRAP) / 100);
    gpio_put(motor->dir_pin, 1 );
    }
    else if (motor->duty < 0){
    level = ((absolute_duty * WRAP) / 100);
    gpio_put(motor->dir_pin, 0);
    }
    
    pwm_set_gpio_level(motor->pwm_pin, level);
}


void pin_init() {
    // Left motor setup
    gpio_set_function(left.pwm_pin, GPIO_FUNC_PWM);
    gpio_init(left.dir_pin);
    gpio_set_dir(left.dir_pin, GPIO_OUT);
    
    // Right motor setup
    gpio_set_function(right.pwm_pin, GPIO_FUNC_PWM);
    gpio_init(right.dir_pin);
    gpio_set_dir(right.dir_pin, GPIO_OUT);

    // Configure PWM slices properly
    pwm_config config = pwm_get_default_config();
    float div = 250.0;
    unsigned int wrap = 99;
    
    // PWM configuration
    uint slice_num_l = pwm_gpio_to_slice_num(left.pwm_pin);
    pwm_config_set_clkdiv(&config, div);
    pwm_config_set_wrap(&config, wrap);
    pwm_init(slice_num_l, &config, true);

    uint slice_num_r = pwm_gpio_to_slice_num(right.pwm_pin); 
    pwm_config_set_clkdiv(&config, div);
    pwm_config_set_wrap(&config, wrap);
    pwm_init(slice_num_r, &config, true);
}


int main() {
    stdio_init_all();
    pin_init();

    while (!stdio_usb_connected()){
        ;
    }
    sleep_ms(50);
    printf("Start");
    
    printf("Motor Control Ready\n");
    printf("Use:\n");
    printf("  q/a - Left +/- \n");
    printf("  e/d - Right +/-\n");
    printf("Current Duty (L/R):   0%%    0%%\r");

    while(1) {
        int c = getchar_timeout_us(0);
        switch(c) {
            case 'q':
                if(left.duty < DUTY_MAX) left.duty++;
                break;
            case 'a':
                if(left.duty > DUTY_MIN) left.duty--;
                break;
            case 'e':
                if(right.duty < DUTY_MAX) right.duty++;
                break;
            case 'd':
                if(right.duty > DUTY_MIN) right.duty--;
                break;
        }
            // Update motor outputs
            update_motor(&left);
            update_motor(&right);
            
            // Print status
            printf("Current Duty (L/R): %3d%%  %3d%%\r", 
                  left.duty, right.duty);
        sleep_ms(10);
    }
}
