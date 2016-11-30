#ifndef ACCEL_H
#define ACCEL_H

#include <stdint.h>

/* LIS2HH12 Registers */
#define WHO_AM_I        0x0F
#define TEMP_L          0x0B
#define TEMP_H          0x0C
#define CTRL1           0x20
#define CTRL2           0x21
#define CTRL3           0x22
#define CTRL4           0x24
#define CTRL5           0x24
#define CTRL6           0x25
#define CTRL7           0x26
#define STATUS          0x27
#define OUT_X_L         0x28
#define OUT_X_H         0x29
#define OUT_Y_L         0x2A
#define OUT_Y_H         0x2B
#define OUT_Z_L         0x2C
#define OUT_Z_H         0x2D
#define FIFO_CTRL       0x2E
#define FIFO_SRC        0x2F
#define XL_REFERENCE    0x3A
#define XH_REFERENCE    0x3B
#define YL_REFERENCE    0x3C
#define YH_REFERENCE    0x3D
#define ZL_REFERENCE    0x3F
#define ZH_REFERENCE    0x3F

struct {
    uint16_t x;
    uint16_t y;
    uint16_t z;
} typedef accel_t;

void accelerometer_init(void);
void get_acceleration(accel_t* a);
void write_register(uint8_t address, uint8_t data);

#endif
