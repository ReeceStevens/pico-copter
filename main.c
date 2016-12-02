#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "PLL.h"
#include "ssi.h"
#include "UART.h"
#include "accelerometer.h"
#include "motor.h"

#define START_SPEED 500

accel_t accel_frame;
accel_t prev_accel_frame;

/*
 *  p_controller_update()
 *
 *  Set new motor speeds: M1, M3 left side; M1, M2 top.
 *   +x axis is right, +y is bottom of the board, +z is up into the air
 *   Δx -> left (-)/right (+)
 *   Δy -> top (-)/bottom (+)
 *   Δz -> down (-)/up (+)
 */
void p_controller_update(void) {
    //P -- map the Gs to the motor duty cycle (0-1000)
    uint32_t Px = (4095-accel_frame.x)*1000/4095;
    uint32_t Py = (4095-accel_frame.y)*1000/4095;
    uint32_t Pz = (4095-accel_frame.z)*1000/4095;

    uint16_t M1_speed = (0 -Px -Py +Pz)/3;
    uint16_t M2_speed = (   Px -Py +Pz)/3;
    uint16_t M3_speed = (0 -Px +Py +Pz)/3;
    uint16_t M4_speed = (   Px +Py +Pz)/3;

    if(M1_speed < 0){
        M1_speed = 0;
    }else if(M1_speed > 1000){
        M1_speed = 1000;
    }

    if(M2_speed < 0){
        M2_speed = 0;
    }else if(M2_speed > 1000){
        M2_speed = 1000;
    }

    if(M3_speed < 0){
        M3_speed = 0;
    }else if(M3_speed > 1000){
        M3_speed = 1000;
    }

    if(M4_speed < 0){
        M4_speed = 0;
    }else if(M4_speed > 1000){
        M4_speed = 1000;
    }

    set_motor(M1, M1_speed);
    set_motor(M2, M2_speed);
    set_motor(M3, M3_speed);
    set_motor(M4, M4_speed);
}

int main(void)
{
    volatile uint32_t ui32Loop;

    PLL_Init(Bus80MHz);

    UART_Init();
    printf("System Initializing...\n\n");
    accelerometer_init();
    motor_init_all();
    printf("Init complete, starting up picocopter!\n\n");

    get_acceleration(&accel_frame);
    set_all_motors(START_SPEED);

    while(1) {
        //
        // Delay for a bit.
        //
        for(ui32Loop = 0; ui32Loop < 200000; ui32Loop++){}

        prev_accel_frame = accel_frame;
        get_acceleration(&accel_frame);

        #ifdef DEBUG
            printf("Acceleration data:\n\n");
            UART_OutString("X: ");
            UART_OutUDec(accel_frame.x);
            UART_OutString("\nY: ");
            UART_OutUDec(accel_frame.y);
            UART_OutString("\nZ: ");
            UART_OutUDec(accel_frame.z);
            UART_OutString("\n");
        #endif

        //P controller (ignoring integral and derivative components
        //M1 & M4 are opposites and spin Clockwise to give +z acceleration
        //Gives acceleration in 12 bit number on scale of 0-4095, mapped to -2 to +2 g
        //For ALL motors, values are calculated in the same way;
        // BUT motors 2 and 3 are wired in the opposite direction for simplicity in code
        p_controller_update();

    }
}
