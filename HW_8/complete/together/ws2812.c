#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "ws2812.pio.h"

#define IS_RGBW false
#define NUM_PIXELS 4
 
#ifdef PICO_DEFAULT_WS2812_PIN
#define WS2812_PIN PICO_DEFAULT_WS2812_PIN
#else
 // default to pin 2 if the board doesn't have a default WS2812 pin defined
#define WS2812_PIN 11
#endif
 
// Check the pin is compatible with the platform
#if WS2812_PIN >= NUM_BANK0_GPIOS
#error Attempting to use a pin>=32 on a platform that does not support it
#endif

#define MOTOR_PIN 17

typedef struct {  
    float servo_angle; 
    bool increasing;   
} State;

// link three 8bit colors together
typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} wsColor; 
 
// adapted from https://forum.arduino.cc/index.php?topic=8498.0
// hue is a number from 0 to 360 that describes a color on the color wheel
// sat is the saturation level, from 0 to 1, where 1 is full color and 0 is gray
// brightness sets the maximum brightness, from 0 to 1
wsColor HSBtoRGB(float hue, float sat, float brightness) {
    float red = 0.0;
    float green = 0.0;
    float blue = 0.0;

    if (sat == 0.0) {
        red = brightness;
        green = brightness;
        blue = brightness;
    } else {
        if (hue == 360.0) {
            hue = 0;
        }

        int slice = hue / 60.0;
        float hue_frac = (hue / 60.0) - slice;

        float aa = brightness * (1.0 - sat);
        float bb = brightness * (1.0 - sat * hue_frac);
        float cc = brightness * (1.0 - sat * (1.0 - hue_frac));

        switch (slice) {
            case 0:
                red = brightness;
                green = cc;
                blue = aa;
                break;
            case 1:
                red = bb;
                green = brightness;
                blue = aa;
                break;
            case 2:
                red = aa;
                green = brightness;
                blue = cc;
                break;
            case 3:
                red = aa;
                green = bb;
                blue = brightness;
                break;
            case 4:
                red = cc;
                green = aa;
                blue = brightness;
                break;
            case 5:
                red = brightness;
                green = aa;
                blue = bb;
                break;
            default:
                red = 0.0;
                green = 0.0;
                blue = 0.0;
                break;
        }
    }

    unsigned char ired = red * 255.0;
    unsigned char igreen = green * 255.0;
    unsigned char iblue = blue * 255.0;

    wsColor c;
    c.r = ired;
    c.g = igreen;
    c.b = iblue;
    return c;
}

static inline void put_pixel(PIO pio, uint sm, uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 16) |
            ((uint32_t) (g) << 8) |
            (uint32_t) (b);
}

void set_servo_angle(float angle) {
    uint16_t wrap = 46875;
    float pulse_min = 0.5;  
    float pulse_max = 2.5;  
    float pulse_width = pulse_min + (angle / 180.0) * (pulse_max - pulse_min);
    uint16_t duty = (pulse_width / 20) * wrap;
    pwm_set_gpio_level(MOTOR_PIN, duty);
}

int main() {
    //set_sys_clock_48();
    stdio_init_all();
    printf("WS2812 Smoke Test, using pin %d\n", WS2812_PIN);

    // todo get free sm
    PIO pio;
    uint sm;
    uint offset;

    // This will find a free pio and state machine for our program and load it for us
    // We use pio_claim_free_sm_and_add_program_for_gpio_range (for_gpio_range variant)
    // so we will get a PIO instance suitable for addressing gpios >= 32 if needed and supported by the hardware
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, WS2812_PIN, 1, true);
    hard_assert(success);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    gpio_set_function(MOTOR_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(MOTOR_PIN);

    float div = 64.0;          
    uint16_t wrap = 46875; //150 /(div + freq)
    pwm_set_clkdiv(slice_num, div);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true);


    float base_hue = 0.0f;
    State state = {
        .servo_angle = 0.0f,
        .increasing = true
    };

    while (true) {

    for (int i = 0; i < NUM_PIXELS; i++) {
        float led_hue = fmod(base_hue + (i * 90.0f), 360.0f);
        wsColor color = HSBtoRGB(led_hue, 1.0f, 0.5f);  
        put_pixel(pio, sm, urgb_u32(color.r, color.g, color.b));
    }

        base_hue = fmod(base_hue + 1.44f, 360.0f);

    // Update servo position
    if (state.increasing) {
        state.servo_angle += 0.72f; 
        if (state.servo_angle >= 180.0f) state.increasing = false;
    } else {
        state.servo_angle -= 0.72f;
        if (state.servo_angle <= 0.0f) state.increasing = true;
    }
    set_servo_angle(state.servo_angle);
    
    sleep_ms(20);
    }
}

