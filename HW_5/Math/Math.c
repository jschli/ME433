#include <stdio.h>
#include "pico/stdlib.h"

#define ITERATIONS 1000

int main() {
    stdio_init_all();
    
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Start!\n");

    volatile float f1, f2;
    volatile float f_add, f_sub, f_mult, f_div;

    printf("Enter two floats to use: ");
    scanf("%f %f", (float*)&f1, (float*)&f2);  // Cast to fix volatile issue

    // Addition
    absolute_time_t t1 = get_absolute_time();
    uint64_t t_start = to_us_since_boot(t1);
    for(uint16_t i = 0; i < ITERATIONS; i++) {
        f_add = f1 + f2;
    }
    absolute_time_t t2 = get_absolute_time();
    uint64_t t_elapsed = to_us_since_boot(t2) - t_start;
    float avg_us = t_elapsed / (float)ITERATIONS;
    printf("\nAddition:\n");
    printf("Total time: %llu µs\n", t_elapsed);
    printf("Avg/operation: %.3f µs\n", avg_us);
    printf("Cycles: %.0f\n", avg_us / 0.006667f);

    // Subtraction
    t1 = get_absolute_time();
    t_start = to_us_since_boot(t1);
    for(uint16_t i = 0; i < ITERATIONS; i++) {
        f_sub = f1 - f2;
    }
    t2 = get_absolute_time();
    t_elapsed = to_us_since_boot(t2) - t_start;
    avg_us = t_elapsed / (float)ITERATIONS;
    printf("\nSubtraction:\n");
    printf("Total time: %llu µs\n", t_elapsed);
    printf("Avg/operation: %.3f µs\n", avg_us);
    printf("Cycles: %.0f\n", avg_us / 0.006667f);

    // Multiplication
    t1 = get_absolute_time();
    t_start = to_us_since_boot(t1);
    for(uint16_t i = 0; i < ITERATIONS; i++) {
        f_mult = f1 * f2;
    }
    t2 = get_absolute_time();
    t_elapsed = to_us_since_boot(t2) - t_start;
    avg_us = t_elapsed / (float)ITERATIONS;
    printf("\nMultiplication:\n");
    printf("Total time: %llu µs\n", t_elapsed);
    printf("Avg/operation: %.3f µs\n", avg_us);
    printf("Cycles: %.0f\n", avg_us / 0.006667f);

    // Division
    t1 = get_absolute_time();
    t_start = to_us_since_boot(t1);
    for(uint16_t i = 0; i < ITERATIONS; i++) {
        f_div = f1 / f2;
    }
    t2 = get_absolute_time();
    t_elapsed = to_us_since_boot(t2) - t_start;
    avg_us = t_elapsed / (float)ITERATIONS;
    printf("\nDivision:\n");
    printf("Total time: %llu µs\n", t_elapsed);
    printf("Avg/operation: %.3f µs\n", avg_us);
    printf("Cycles: %.0f\n", avg_us / 0.006667f);

    return 0;
}
