#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9
#define LED_C 15
#define CYW43_WL_GPIO_LED_PIN
#define MCP_adress 0b0100000
#define IODIR 0x00
#define GPIO 0x09

void setPin(unsigned char, unsigned char, unsigned char);
unsigned char readPin(unsigned char, unsigned char);



int main()
{
    stdio_init_all();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    setPin(MCP_adress, IODIR, 0b00000001);
    setPin(MCP_adress, GPIO, 0b00000000);

    bool led_state = false;
    bool led_on = false;
    bool button_pressed = false;

    //1. Step: Initialization + LED

    while (true) {
        gpio_put(PICO_DEFAULT_LED_PIN, led_state);
        led_state = !led_state;
        
        unsigned char gpio_state = readPin(MCP_adress, GPIO);

        if ((gpio_state & 0b00000001) == 0b00000000) {
            setPin(MCP_adress, GPIO, 0b10000000);  // Turn LED ON (GP7=1)
        } else {
            setPin(MCP_adress, GPIO, 0b00000000);  // Turn LED OFF (GP7=0)
        }
        sleep_ms(100);
    }
}

void setPin(unsigned char address, unsigned char reg, unsigned char value){
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = value;

    i2c_write_blocking(i2c_default, address, buf, 2, false);

    return;
}

unsigned char readPin(unsigned char address, unsigned char reg){
    uint8_t buf;

    i2c_write_blocking(i2c_default, address, &reg, 1, true);  // true to keep master control of bus
    i2c_read_blocking(i2c_default, address, &buf, 1, false);  // false - finished with bus

    return buf;
}