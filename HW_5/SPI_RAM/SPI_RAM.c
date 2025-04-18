#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <math.h>

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS_DAC 17
#define PIN_CS_RAM 13
#define PIN_SCK  18
#define PIN_MOSI 19

void init_ram();
void ram_write(uint16_t, float);
float ram_read(uint16_t);
void writeDac(float);

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

union FloatInt {
    float f;
    uint32_t i;
};



int main()
{
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Start!\n");

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS_DAC,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_CS_RAM,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS_DAC, GPIO_OUT);
    gpio_put(PIN_CS_DAC, 1);
    gpio_set_dir(PIN_CS_RAM, GPIO_OUT);
    gpio_put(PIN_CS_RAM, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    init_ram();

    for(uint16_t i=0; i<1000; i++){
        float angle = 2*M_PI*i/1000;
        float sin_voltage = 1.65f * sinf(angle) + 1.65f;
        ram_write(i, sin_voltage);
    }

    printf("Done!\n");

    uint16_t counter = 0;

    while (true) {
        float sin_voltage = ram_read(counter);
        printf("Current Voltage at step %u: %.3f V\n", counter, sin_voltage);
        //writeDac(v);
        counter = (counter + 1)% 1000;
        sleep_ms(1000);
    }
}

void init_ram(){

    uint8_t buff[2];
    buff[0] = 0b00000101; //i want to change the status register
    buff[1] = 0b01000000; // to sequential mode


    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, buff, 2); // where data is a uint8_t array with length len
    cs_deselect(PIN_CS_RAM);
}

void ram_write(uint16_t a, float v){
    uint8_t buff[7];

    buff[0] = 0b00000010;
    buff[1] = (a >> 8)&0xFF;
    buff[2] = a&0xFF;

    union FloatInt num;
    num.f = v;

    buff[3] = (num.i >> 24)&0xFF; //leftmost 8bit
    buff[4] = (num.i >> 16)&0xFF;//inner leftmost 8bit
    buff[5] = (num.i >> 8)&0xFF;
    buff[6] = (num.i)&0xFF;
    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, buff, 7); 
    cs_deselect(PIN_CS_RAM);
}

float ram_read(uint16_t a){

    uint8_t out_buff[7], in_buff[7];

    out_buff[0] = 0b00000011;
    out_buff[1] = (a >> 8)&0xFF;
    out_buff[2] = a&0xFF;

    out_buff[3] = 0b00000001;
    out_buff[4] = 0b00000001;
    out_buff[5] = 0b00000001;
    out_buff[6] = 0b00000001;

    cs_select(PIN_CS_RAM);
    spi_write_read_blocking(SPI_PORT, out_buff, in_buff, 7);
    cs_deselect(PIN_CS_RAM);

    union FloatInt num;
    num.i = 0;

    num.i = in_buff[3] << 24 | in_buff[4]<<16 | in_buff[5]<<8 | in_buff[6];

    return num.f;
}

void writeDac(float voltage){
    uint8_t data[2];
    int len = 2;
    uint16_t v = voltage * 1023.0f / 3.3f; 

    uint16_t bits = (0 << 15) | (1 << 14) | (1 << 13) | (1 << 12) | (v << 2) | (0 << 1) | (0 << 0);

    data[0] = (bits >> 8) & 0xFF;
    data[1] = bits & 0xFF;
    cs_select(PIN_CS_DAC);
    spi_write_blocking(SPI_PORT, data, len); // where data is a uint8_t array with length len
    cs_deselect(PIN_CS_DAC);
}