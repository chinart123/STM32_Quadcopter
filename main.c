#include "stm32f10x.h"
#include "button.h"
#include "i2c_mpu_debug.h" 
#include "xx_mpu_data_fusion.h" 
#include "hal_timer_pwm.h" 
#include "pid_control.h"  
#include "telemetry.h"    // <--- NHÚNG MODULE GIAO TIẾP VÀO
#include <string.h>
#include <stdio.h> 

volatile unsigned int global_tick = 0; 
volatile uint8_t xx = 0x00;           
volatile uint8_t xx_mpu_state = 0x00; 
volatile uint8_t xx_mpu_id = 0x00;
uint32_t last_print_tick = 0;

void SysTick_Handler(void) { global_tick++; }

int main(void) {
    // 1. Khởi tạo GPIO Nút bấm
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    GPIOA->CRL &= ~(0xFF << 0);            
    GPIOA->CRL |= (0x88 << 0);             
    GPIOA->ODR |= (1UL << 0) | (1UL << 1); 

    // 2. Khởi tạo Ngoại vi
    I2C1_Init();
    SysTick_Config(SystemCoreClock / 1000); 
    HAL_TIM4_Micros_Init(); 
    Telemetry_Init();       // <--- KHỞI TẠO UART1 LÀM NHIỆM VỤ TELEMETRY
    
    // 3. Khởi tạo Tay chân và Não bộ
    HAL_TIM3_PWM_Init(); 
    PID_Init();          
    
    printf("\n--- HET THONG FLIGHT CONTROLLER (STAGE 8) SAN SANG ---\n");

    xx = 0x00;
    xx_mpu_state = 0x00;
    xx_mpu_id = 0x00;
    MPU_Sleep(); 

    while (1) {
        button_state_hardware_scan();
        button_fsm_process(&btn_PA0, global_tick);
        button_fsm_process(&btn_PA1, global_tick);

        if (btn_PA1.event_code == BTN_EVENT_SINGLE_CLICK) {
            if (xx == 0x00 || xx == 0x04) {
                printf("\nKhoi dong He thong...\n");
                MPU_WakeUp();
                MPU_Fusion_Init(); 
                MPU_Read_WhoAmI(&xx_mpu_state, &xx_mpu_id); 
                
                printf("Dang Calibrate, vui long de im mach...\n");
                if (MPU_Fusion_Calibrate() == 1) {
                    xx = 0x01; 
                    printf("Calibrate XONG! San sang bay (0x02)\n");
                    
                    Telemetry_Print_Header();           // In Header CSV
                    Telemetry_Reset_Counters(global_tick); // Reset đếm I2C
                } else {
                    xx = 0x00; 
                    printf("LOI: Day I2C bi long, khong the Calibrate!\n");
                }
            } 
            else {
                printf("\nDA TAT HE THONG (0x04)!\n");
                MPU_Sleep();       
                memset((void*)&Drone_IMU, 0, sizeof(MPU_Motion_t));
                
                HAL_TIM3_PWM_SetDuty(1, 0); HAL_TIM3_PWM_SetDuty(2, 0);
                HAL_TIM3_PWM_SetDuty(3, 0); HAL_TIM3_PWM_SetDuty(4, 0);
                xx = 0x04;         
            }
            btn_PA1.event_code = BTN_EVENT_NONE; 
        }

        if (btn_PA0.event_code == BTN_EVENT_SINGLE_CLICK) {
            if (xx == 0x01 || xx == 0x03) {
                xx = 0x02; 
                printf("\n--- DONG CO DA MO GA (THROTTLE = 400) ---\n");
            } 
            else if (xx == 0x02) {
                xx = 0x03; 
                HAL_TIM3_PWM_SetDuty(1, 0); HAL_TIM3_PWM_SetDuty(2, 0);
                HAL_TIM3_PWM_SetDuty(3, 0); HAL_TIM3_PWM_SetDuty(4, 0);
            }
            btn_PA0.event_code = BTN_EVENT_NONE; 
        }

        switch (xx) {
            case 0x02:
                if (MPU_Fusion_Read_Burst() == 1) {
                    Telemetry_Update_I2C_Counter(14); // Báo cáo: Kéo thành công 14 bytes
                    MPU_Fusion_Compute();
                    
                    PID_Compute(Drone_IMU.Pitch, -Drone_IMU.Roll, Drone_IMU.Yaw, Drone_IMU.dt); 
                    Motor_Mixer(400); 
                    
                    // XỬ LÝ TELEMETRY GỌN NHẸ
                    Telemetry_Calculate_Speed(global_tick); 
                    Telemetry_Send_CSV(global_tick, &Drone_IMU, TIM3->CCR1, TIM3->CCR2, TIM3->CCR3, TIM3->CCR4);
                }
                break;
                
            case 0x03:
                if (global_tick - last_print_tick >= 1000) {
                    printf("DANG DUNG (0x03) - DONG CO TAT\n");
                    last_print_tick = global_tick;
                }
                break;

            default:
                break;
        }
    }
}