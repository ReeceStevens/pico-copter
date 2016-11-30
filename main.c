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

accel_t accel_frame;

void heartbeat_init(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)){}
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);
}

int
main(void)
{
    volatile uint32_t ui32Loop;

    PLL_Init(Bus80MHz);

    UART_Init();
    printf("System Initializing...\n\n");
    heartbeat_init();
    accelerometer_init();
    motor_init(M1);
    printf("Init complete, starting up the quadcopter!\n\n");

    set_motor(M1, 500);
    //
    // Loop forever.
    //
    while(1)
    {
        //
        // Turn on the LED.
        //
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);

        UART_OutUDec(read_register(WHO_AM_I));
        UART_OutString("\n");
        /* printf("Getting acceleration data:\n\n"); */
        /* get_acceleration(&accel_frame); */
        /* UART_OutString("X: "); */
        /* UART_OutUDec(accel_frame.x); */
        /* UART_OutString("\nY: "); */
        /* UART_OutUDec(accel_frame.y); */
        /* UART_OutString("\nZ: "); */
        /* UART_OutUDec(accel_frame.z); */
        /* UART_OutString("\n"); */

        //
        // Delay for a bit.
        //
        for(ui32Loop = 0; ui32Loop < 200000; ui32Loop++)
        {
        }

        //
        // Turn off the LED.
        //
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x0);

        //
        // Delay for a bit.
        //
        for(ui32Loop = 0; ui32Loop < 200000; ui32Loop++)
        {
        }

    }
}
