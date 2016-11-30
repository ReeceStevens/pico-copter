#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

enum{
    M1,
    M2,
    M3,
    M4
}typedef motor_t;

void motor_init(motor_t motor);
void set_motor(motor_t motor, uint16_t speed);
uint16_t get_motor_speed(motor_t motor);

#endif
