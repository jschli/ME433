#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "font.h"
#include "ssd1306.h"


// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9
#define CYW43_WL_GPIO_LED_PIN
#define MCP_adress 0b0100000
#define ADC_PIN 26

void drawLetter(int x, int y, char c);
void drawMessage(int x, int y, char*m);



int main()
{
    stdio_init_all();
    
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Start!\n");

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    adc_init(); 
    adc_gpio_init(ADC_PIN); 
    adc_select_input(0); 

    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    bool led_state = false;


    while (true) {
        gpio_put(PICO_DEFAULT_LED_PIN, led_state);
        led_state = !led_state;

        absolute_time_t t1 = get_absolute_time();
        uint64_t t_start = to_us_since_boot(t1);

        ssd1306_clear();
        uint16_t raw = adc_read();
        float voltage = raw * 3.3f / 4096;
        char message[50]; 
        sprintf(message, "current voltage = %.3f", voltage); 
        drawMessage(0,0,message); // draw starting at x=20,y=10  
        ssd1306_update();
        absolute_time_t t2 = get_absolute_time();
        uint64_t t_elapsed = to_us_since_boot(t2) - t_start; // in microseconds
         // Calculate FPS
         float fps = 1000000.0f / (float)t_elapsed;
         char fps_message[32];
         sprintf(fps_message, "FPS: %.2f", fps);
         drawMessage(0, 24, fps_message);
 
         ssd1306_update();
 
         printf("Total time: %llu us, FPS: %.2f\n", t_elapsed, fps);

        sleep_ms(10);
    }
}


void drawMessage(int x, int y, char*m){
    int i = 0;
    while (m[i] != 0){
        drawLetter(x+i*5,y,m[i]);
        i++;
    }
}

void drawLetter(int x, int y, char c){
    int j;
    for(j = 0;j<5;j++){
        char col = ASCII[c-0x20][j];
        int i ;
        for(i = 0;i<8;i++){
            char bit = (col >> i)&0b1;
            ssd1306_drawPixel(x+j,y+i, bit);
        }
    }
}