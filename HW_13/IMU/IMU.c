#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

// config registers
#define CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define PWR_MGMT_1 0x6B
#define PWR_MGMT_2 0x6C
// sensor data registers:
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40
#define TEMP_OUT_H   0x41
#define TEMP_OUT_L   0x42
#define GYRO_XOUT_H  0x43
#define GYRO_XOUT_L  0x44
#define GYRO_YOUT_H  0x45
#define GYRO_YOUT_L  0x46
#define GYRO_ZOUT_H  0x47
#define GYRO_ZOUT_L  0x48
#define WHO_AM_I     0x75

#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9

#define MPU_ADDR 0x68

void mpu_write_reg(uint8_t reg, uint8_t val);
uint8_t mpu_read_reg(uint8_t reg);
void mpu_read_bytes(uint8_t reg, uint8_t *buffer, uint8_t len);
void mpu_init();
void draw_line(int x0, int y0, int x1, int y1);



int main()
{
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Start!\n");

    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    mpu_init(); 

    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    while (true) {
        // Read accelerometer data
        uint8_t accel_data[6];
        mpu_read_bytes(ACCEL_XOUT_H, accel_data, 6);
        
        // Convert raw data to integers
        int16_t ax = (accel_data[0] << 8) | accel_data[1];
        int16_t ay = (accel_data[2] << 8) | accel_data[3];
        int16_t az = (accel_data[4] << 8) | accel_data[5];

        // Convert to display coordinates
        float ay_g = ay / 16384.0f;
        float az_g = az / 16384.0f;
        int end_x = 64 - (int)(az_g * 64);  
        int end_y = 16 - (int)(ay_g * 16); 
        
        printf("Accel: X:%.2fg\tY:%.2fg\tZ:%.2fg\tend_x:%d\tend_y:%d\n", 
                (float)ax/16384.0, 
                (float)ay/16384.0,
                (float)az/16384.0,
                end_x,
                end_y);

        // Update display
        ssd1306_clear();
        draw_line(64, 16, end_x, end_y);
        ssd1306_update();
        sleep_ms(500);
    }

}

void mpu_init() {
    mpu_write_reg(PWR_MGMT_1, 0x00);
    
}

void mpu_write_reg(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    i2c_write_blocking(I2C_PORT, MPU_ADDR, buf, 2, false);
}

uint8_t mpu_read_reg(uint8_t reg) {
    uint8_t val;
    i2c_write_blocking(I2C_PORT, MPU_ADDR, &reg, 1, true); 
    i2c_read_blocking(I2C_PORT, MPU_ADDR, &val, 1, false); 
    return val;
}

void mpu_read_bytes(uint8_t reg, uint8_t *buffer, uint8_t len) {
    i2c_write_blocking(I2C_PORT, MPU_ADDR, &reg, 1, true); 
    i2c_read_blocking(I2C_PORT, MPU_ADDR, buffer, len, false);
}

void draw_line(int x0, int y0, int x1, int y1) {
    //x
    if (x1 > x0){
        for (int i = x0; i <= x1; i++) {
            ssd1306_drawPixel(i, y0, true);
        }
    }
    if (x1 < x0){
        for (int i = x1; i <= x0; i++) {
            ssd1306_drawPixel(i, y0, true);
        }
    }
    //y
    if (y1 > y0){
        for (int i = y0; i <= y1; i++) {
            ssd1306_drawPixel(x0, i, true);
        }
    }
    if (y1 < y0){
        for (int i = y1; i <= y0; i++) {
            ssd1306_drawPixel(x0, i, true);
        }
    }
}