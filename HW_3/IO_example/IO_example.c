#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define ADC_PIN 26
#define BUTTON_PIN 15

int main() {
    stdio_init_all();

    //inits
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_down(BUTTON_PIN); 
    adc_init(); 
    adc_gpio_init(ADC_PIN); 
    adc_select_input(0); 

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Start!\n");

    // Turn on LED
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    printf("LED ON - press button to continue\n");

    // Wait for button press
    while (!gpio_get(BUTTON_PIN)) {
        sleep_ms(10);
    }
    
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    printf("Button pressed - LED OFF\n");
 
    // Main program loop
    while (1) {
        int num_samples = 0;
        printf("Enter number of samples (1-100): ");
        scanf("%d", &num_samples);
        printf("Reading %d samples:\n", num_samples);
        for(int i = 0; i < num_samples; i++) {
            uint16_t raw = adc_read();
            float voltage = raw * 3.3f / 4096;
            printf("Sample %3d: %.3f V\n", i+1, voltage);
        }
    }
}
