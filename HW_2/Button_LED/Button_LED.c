#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define LED_PIN 15
#define BUTTON_PIN 13

volatile uint32_t press_count = 0;
volatile bool led_state = false;

void gpio_callback(uint gpio, uint32_t events) {
    
        press_count++;
        led_state = !led_state;
        gpio_put(LED_PIN, led_state);
        printf("Button pressed %d times\n", press_count);
        sleep_ms(1000);
    
}

int main()
{
    stdio_init_all();

    printf("Hello GPIO IRQ\n");

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_down(BUTTON_PIN); 


    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    while (true) {

    }
}
