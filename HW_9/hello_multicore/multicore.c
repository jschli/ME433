/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"

#define FLAG_VALUE 123
#define LED_PIN 15
#define ADC_PIN 26

volatile float data_10;

void core1_entry() {

    multicore_fifo_push_blocking(FLAG_VALUE);

    // init an led output pin
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);
    // init adc
    adc_init(); 
    adc_gpio_init(ADC_PIN); 
    adc_select_input(0); 

    while (1){
        //pop form core 0 
        uint32_t command = multicore_fifo_pop_blocking();
        if(command == 0){
            // read adc
            uint16_t raw = adc_read();
            float voltage = raw * 3.3f / 4096;
            // put the adc value into data_10
            data_10 = voltage;
            // push a command to core 0
        }
        if( command == 1){
            gpio_put(LED_PIN, true);
        }
        if( command == 2){
            gpio_put(LED_PIN, false);
        }

        multicore_fifo_push_blocking(FLAG_VALUE);
    }

}

int main() {
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    printf("Hello, multicore!\n");

    /// \tag::setup_multicore[]

    multicore_launch_core1(core1_entry);

    // Wait for it to start up

    uint32_t command = multicore_fifo_pop_blocking();

    if (command != FLAG_VALUE)
        printf("Hmm, that's not right on core 0!\n");
    else {
        multicore_fifo_push_blocking(FLAG_VALUE);
        printf("It's all gone well on core 0!");
    }

    while (1){
        // printf user intstruction
        printf("0: return the voltage on pin A0\n 1: turn on an LED on GP15\n 2: turn off an LED on GP15\n");
        // scanf to get the instruction
        int command;
        scanf("%d", &command);
        // printf the command back
        if (command >= 0 && command <= 2){
            printf("Performing %d \n", command);
            // push to core 1 a command
            multicore_fifo_push_blocking(command);
            // pop from core 1
            multicore_fifo_pop_blocking();
            // print the value depending on what you get from pop
            if (command == 0){
                printf("Voltage: %.2fV\n", data_10);
            }

        }
        else{
            printf(" %d is an invalid command!\n", command);
        }
    }
        tight_loop_contents();

    /// \end::setup_multicore[]
}
