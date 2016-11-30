#include <stdio.h>
#include "ssi.h"
#include "accelerometer.h"

#define PB6             (*((volatile uint32_t *)0x40005100))

#define KNOWN_IDENTITY_SEQUENCE 0x41

static accel_t unsafe_accel_frame;

uint8_t read_register(uint8_t address) {
    /* PB6 = 0x00; // CS Low */
    uint8_t address_packet = address | 0x80;
    ssi_transfer(address_packet);
    uint8_t retval = ssi_transfer(0x00);
    /* PB6 = 0x40; // CS High */
    return retval; // Send a dummy signal to get back information
}

void write_register(uint8_t address, uint8_t data) {
    /* PB6 = 0x00; // CS Low */
    uint8_t address_packet = address & ~(0x80);
    ssi_transfer(address_packet);
    ssi_transfer(data);
    /* PB6 = 0x40; // CS High */
}

static void data_ready_interrupt_init(void) {
  // Port B Init (PB7)
  SYSCTL_RCGCGPIO_R |= 0x02;        // 1) activate clock for Port B
  while((SYSCTL_PRGPIO_R&0x02)==0){}; // allow time for clock to start
  GPIO_PORTB_AFSEL_R  &= ~0x02; // no AF
  GPIO_PORTB_DIR_R &= ~0x02; // in PB1
  GPIO_PORTB_PCTL_R &= ~0x000000F0;
  GPIO_PORTB_AMSEL_R &= ~0x02;
  GPIO_PORTB_DEN_R |= 0x02; // enable PB1

  GPIO_PORTB_AMSEL_R = 0;               // disable analog functionality on PB
  GPIO_PORTB_IS_R &= ~0x02;             // Clear IS
  GPIO_PORTB_IBE_R &= ~0x02;            // Do not interrupt both edges
  GPIO_PORTB_IEV_R |= 0x02;             // Interrupt on rising edge
  GPIO_PORTB_ICR_R = 0x02;              // Clear flags
  GPIO_PORTB_IM_R |= 0x02;              // Enable interrupts
  NVIC_PRI0_R = (NVIC_PRI0_R&0xFFFFFFFF)|0x0000A000; // (g) priority 5
  NVIC_EN0_R |= 0x2;      // (h) enable interrupt 1 in NVIC
}

void accelerometer_init(void) {
    volatile uint32_t ui32Loop;
    ssi_init();
    write_register(CTRL5, 0x40); // Reboot accelerometer
    for(ui32Loop = 0; ui32Loop < 8000000; ui32Loop++)
    {
    }
    // Disable I2C, enable SPI 4 wire (CTRL4)
    uint8_t ctrl4_data = read_register(CTRL4);
    write_register(CTRL4, (ctrl4_data | 0x02));
    uint8_t identity_code = read_register(WHO_AM_I);
    if (identity_code != KNOWN_IDENTITY_SEQUENCE) {
        printf("Problem: Accelerometer not properly wired.\n\n");
    }
    // Set ODR to 50 Hz data output rate (CTRL1)
    uint8_t ctrl1_data = read_register(CTRL1);
    write_register(CTRL1, (ctrl1_data & ~(0x70)) | 0x20);
    // Start by bypassing HPF (default, no action required) (CTRL2)
    // Turn FIFO off, set data ready for INT1 (CTRL3)
    uint8_t ctrl3_data = read_register(CTRL3);
    write_register(CTRL3, (ctrl3_data | 0x01));
    unsafe_accel_frame.x = 0;
    unsafe_accel_frame.y = 0;
    unsafe_accel_frame.z = 0;
    data_ready_interrupt_init();
}

void get_acceleration(accel_t* a) {
    NVIC_EN0_R &= ~(0x2); // Disable Data Ready interrupt
    a->x = unsafe_accel_frame.x;
    a->y = unsafe_accel_frame.y;
    a->z = unsafe_accel_frame.z;
    NVIC_EN0_R |= 0x2; // Enable Data Ready interrupt
}

static void update_acceleration(accel_t* a) {
    a->x = (read_register(OUT_X_H) << 8) + read_register(OUT_X_L);
    a->y = (read_register(OUT_Y_H) << 8) + read_register(OUT_Y_L);
    a->z = (read_register(OUT_Z_H) << 8) + read_register(OUT_Z_L);
}

void GPIOPortB_Handler(void) {
    update_acceleration(&unsafe_accel_frame);
    GPIO_PORTB_ICR_R = 0xFF;
}
