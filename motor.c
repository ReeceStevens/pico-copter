#include <stdio.h>

#include "inc/tm4c123gh6pm.h"
#include "motor.h"

#define PB6             (*((volatile uint32_t *)0x40005100)) //M1, PWM0, M0PWM0
#define PB4             (*((volatile uint32_t *)0x40008040)) //M2, PWM1, M0PWM2
#define PE4             (*((volatile uint32_t *)0x40005040)) //M3, PWM2, M0PWM4
#define PC5             (*((volatile uint32_t *)0x40006080)) //M4, PWM3, M0PWM7

#define PERIOD             1000

// Speed represents the speed of the motor as a PWM duty scycle percentage
// Range = 0 to 1000 (tenth of a percentage precision)
// 0 -> full reverse
// 1000 -> full forward
static uint16_t M1_speed;
static uint16_t M2_speed;
static uint16_t M3_speed;
static uint16_t M4_speed;

// The PWMx_init functions all set up PWM pins
// Period of PERIOD, initial duty cycle of 50%
// See pg 312 for sample init code, or Valvanoware, project PWM_4C123
//Also: Datasheet, p1230, p1240,

// PB6 = PWM0_0
static void M1_PWM_Init(){
  SYSCTL_RCGCPWM_R |= 0x01;             // 1) activate PWM0
  SYSCTL_RCGCGPIO_R |= 0x02;            // 2) activate port B
  while((SYSCTL_PRGPIO_R&0x02) == 0){};
  GPIO_PORTB_AFSEL_R |= 0x40;           // enable alt funct on PB6
  GPIO_PORTB_PCTL_R &= ~0x0F000000;     // configure PB6 as PWM0
  GPIO_PORTB_PCTL_R |= 0x04000000;
  GPIO_PORTB_AMSEL_R &= ~0x40;          // disable analog functionality on PB6
  GPIO_PORTB_DEN_R |= 0x40;             // enable digital I/O on PB6
  SYSCTL_RCC_R = 0x00100000 |           // 3) use PWM divider
      (SYSCTL_RCC_R & (~0x000E0000));   //    configure for /2 divider
  PWM0_0_CTL_R = 0;                     // 4) re-loading down-counting mode
  PWM0_0_GENA_R = 0xC8;                 // low on LOAD, high on CMPA down
  // PB6 goes low on LOAD
  // PB6 goes high on CMPA down
  PWM0_0_LOAD_R = PERIOD - 1;           // 5) cycles needed to count down to 0
  PWM0_0_CMPA_R = (PERIOD >> 1) - 1;    // 6) count value when output rises
  PWM0_0_CTL_R |= 0x00000001;           // 7) start PWM0
  PWM0_ENABLE_R |= 0x00000001;          // enable PB6/M0PWM0
}
// change duty cycle of PB6
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
static void M1_PWM_Duty(uint16_t duty){
  PWM0_0_CMPA_R = duty - 1;             // 6) count value when output rises
}

// PB4 = PWM0_1
static void M2_PWM_Init(){
  SYSCTL_RCGCPWM_R |= 0x02;
  SYSCTL_RCGCGPIO_R |= 0x02;
  while((SYSCTL_PRGPIO_R&0x02) == 0){};
  GPIO_PORTB_AFSEL_R |= 0x10;
  GPIO_PORTB_PCTL_R &= ~0x000F0000;
  GPIO_PORTB_PCTL_R |= 0x00040000;
  GPIO_PORTB_AMSEL_R &= ~0x10;
  GPIO_PORTB_DEN_R |= 0x10;
  SYSCTL_RCC_R = 0x00100000 | (SYSCTL_RCC_R & (~0x000E0000));    //?
  PWM0_1_CTL_R = 0;
  PWM0_1_GENA_R = 0xC8;
  PWM0_1_LOAD_R = PERIOD - 1;
  PWM0_1_CMPA_R = (PERIOD >> 1) - 1;
  PWM0_1_CTL_R |= 0x00000001;
  PWM0_ENABLE_R |= 0x00000001;
}

static void M2_PWM_Duty(uint16_t duty){
  PWM0_1_CMPA_R = duty - 1;
}

// PE4 = PWM1_1
static void M3_PWM_Init(){
  SYSCTL_RCGCPWM_R |= 0x04;
  SYSCTL_RCGCGPIO_R |= 0x10;
  while((SYSCTL_PRGPIO_R&0x02) == 0){};
  GPIO_PORTE_AFSEL_R |= 0x10;
  GPIO_PORTE_PCTL_R &= ~0x000F0000;
  GPIO_PORTE_PCTL_R |= 0x00040000;
  GPIO_PORTE_AMSEL_R &= ~0x10;
  GPIO_PORTE_DEN_R |= 0x10;
  SYSCTL_RCC_R = 0x00100000 | (SYSCTL_RCC_R & (~0x000E0000));
  PWM1_1_CTL_R = 0;
  PWM1_1_GENA_R = 0xC8;
  PWM1_1_LOAD_R = PERIOD - 1;
  PWM1_1_CMPA_R = (PERIOD >> 1) - 1;
  PWM1_1_CTL_R |= 0x00000001;
  PWM1_ENABLE_R |= 0x00000001;
}

static void M3_PWM_Duty(uint16_t duty){
  PWM1_1_CMPA_R = duty - 1;
}

// PC5 = PWM0_3
static void M4_PWM_Init(){
  SYSCTL_RCGCPWM_R |= 0x08;
  SYSCTL_RCGCGPIO_R |= 0x04;
  while((SYSCTL_PRGPIO_R&0x02) == 0){};
  GPIO_PORTC_AFSEL_R |= 0x20;
  GPIO_PORTC_PCTL_R &= ~0x00F00000;
  GPIO_PORTC_PCTL_R |= 0x00400000;
  GPIO_PORTC_AMSEL_R &= ~0x20;
  GPIO_PORTC_DEN_R |= 0x20;
  SYSCTL_RCC_R = 0x00100000 | (SYSCTL_RCC_R & (~0x000E0000));
  PWM0_3_CTL_R = 0;
  PWM0_3_GENA_R = 0xC8;
  PWM0_3_LOAD_R = PERIOD - 1;
  PWM0_3_CMPA_R = (PERIOD >> 1) - 1;
  PWM0_3_CTL_R |= 0x00000001;
  PWM0_ENABLE_R |= 0x00000001;
}

static void M4_PWM_Duty(uint16_t duty){
  PWM0_3_CMPA_R = duty - 1;
}


void motor_init(motor_t motor){
    switch(motor){
        case M1:
            M1_PWM_Init();
            break;
        case M2:
            M2_PWM_Init();
            break;
        case M3:
            M3_PWM_Init();
            break;
        case M4:
            M4_PWM_Init();
            break;
        default:
            printf("Problem: The motor you are trying to init is not one of the valid motors.\n\n");
            break;
    }
}

void set_motor(motor_t motor, uint16_t speed){
    switch(motor){
        case M1:
            M1_PWM_Duty(speed);
            M1_speed = speed;
            break;
        case M2:
            M2_PWM_Duty(speed);
            M2_speed = speed;
            break;
        case M3:
            M3_PWM_Duty(speed);
            M3_speed = speed;
            break;
        case M4:
            M4_PWM_Duty(speed);
            M4_speed = speed;
            break;
        default:
            printf("Problem: The motor you are trying to set is not one of the valid motors.\n\n");
            break;
    }
}

uint16_t get_motor_speed(motor_t motor){
    switch(motor){
        case M1:
            return M1_speed;
            break;
        case M2:
            return M2_speed;
            break;
        case M3:
            return M3_speed;
            break;
        case M4:
            return M4_speed;
            break;
        default:
            printf("Problem: The motor you are trying to get is not one of the valid motors.\n\n");
            break;
    }
    return 0;
}

void motor_init_all(void) {
    motor_init(M1);
    motor_init(M2);
    motor_init(M3);
    motor_init(M4);
}

void set_all_motors(uint16_t speed) {
    set_motor(M1, speed);
    set_motor(M2, speed);
    set_motor(M3, speed);
    set_motor(M4, speed);
}
