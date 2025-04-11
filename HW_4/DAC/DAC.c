#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <math.h>

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

void writeDac(int, float);


static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop"); // FIXME
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop"); // FIXME
}



int main()
{
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    float t = 0.0f;

    while (true) {
        float sin_voltage = 1.65f * sinf(2*M_PI*2*t) + 1.65f;

        writeDac(0, sin_voltage);

        float triangular = 3.3f*(2*fabsf(t-floorf(t+0.5f)));

        writeDac(1, triangular);

        t += 0.01f;
        sleep_ms(10);
    }
}

void writeDac(int channel, float voltage){
    uint8_t data[2];
    int len = 2;
    uint16_t v = voltage * 1023.0f / 3.3f; 

    uint16_t bits = (channel << 15) | (1 << 14) | (1 << 13) | (1 << 12) | (v << 2) | (0 << 1) | (0 << 0);

    data[0] = (bits >> 8) & 0xFF;
    data[1] = bits & 0xFF;
    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, len); // where data is a uint8_t array with length len
    cs_deselect(PIN_CS);
}