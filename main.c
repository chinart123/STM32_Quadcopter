#include "stm32f10x.h"
#include "button.h"
#include "xx_mpu_complementary_filter.h"

volatile unsigned int global_tick = 0; 
volatile uint8_t xx = 0x00;           

void SysTick_Handler(void) { global_tick++; }

int main(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    GPIOA->CRL &= ~(0xFF << 0);            
    GPIOA->CRL |= (0x88 << 0);             
    GPIOA->ODR |= (1UL << 0) | (1UL << 1); 
    
    SysTick_Config(SystemCoreClock / 1000); 
    
    // Bật chức năng Printf ra Terminal qua chân PA9
    Filter_Machine_UART_Init();
    printf("--- HET THONG QUADCOPTER SAN SANG ---\n");

    xx = 0x00;
    unsigned int last_print_tick = 0; // Đồng hồ riêng để in màn hình

    while (1) {
        button_state_hardware_scan();
        button_fsm_process(&btn_PA0, global_tick);
        button_fsm_process(&btn_PA1, global_tick);

        if (btn_PA1.event_code == BTN_EVENT_SINGLE_CLICK) {
            if (xx == 0x00 || xx == 0x04) {
                printf("Khoi tao I2C va Timer...\n");
                Filter_Machine_Init(); 
                
                printf("Dang Calibrate, de im mach...\n");
                if (Filter_Machine_Calibrate() == 1) {
                    xx = 0x01; 
                    printf("Calibrate XONG! Offset AZ = %d\n", Filter_IMU.Accel_Z_Offset);
                } else {
                    printf("LOI: Day I2C bi long!\n");
                    xx = 0x00; 
                }
            } else {
                xx = 0x04;         
                printf("Da Tat He Thong (0x04).\n");
            }
            btn_PA1.event_code = BTN_EVENT_NONE; 
        }

        if (btn_PA0.event_code == BTN_EVENT_SINGLE_CLICK) {
            if (xx == 0x01 || xx == 0x03) xx = 0x02; 
            else if (xx == 0x02) xx = 0x03; 
            btn_PA0.event_code = BTN_EVENT_NONE; 
        }

        switch (xx) {
            case 0x02:
                // Hút data và tính toán cực tốc
                if (Filter_Machine_Run() == 1) {
                    
                    // CHỈ PRINT RA MÀN HÌNH MỖI 1 GIÂY ĐỂ TRÁNH NGHẼN CỔ CHAI!
                    if (global_tick - last_print_tick >= 1000) {
                        printf("Speed: %4d Bytes/s | R: %6.2f | P: %6.2f | Y: %6.2f\n", 
                               Filter_IMU.Bytes_Per_Second, 
                               Filter_IMU.Roll, 
                               Filter_IMU.Pitch, 
                               Filter_IMU.Yaw);
                        last_print_tick = global_tick;
                    }
                }
                break;
            default: break;
        }
    }
}