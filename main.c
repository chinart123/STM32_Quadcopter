#include "stm32f10x.h"
#include "button.h"
#include "i2c_mpu_debug.h" 
#include "xx_mpu_data_fusion.h" 
#include "hal_timer_pwm.h" 
#include "pid_control.h"  // <--- NHÚNG NÃO BỘ PID VÀO
#include <string.h>
#include <stdio.h> 

// ==========================================================
// TÍCH HỢP RADAR ĐO LƯU LƯỢNG (UART1 - Chân PA9)
// ==========================================================
void UART1_Debug_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPAEN;
    GPIOA->CRH &= ~(0xFF << 4); 
    GPIOA->CRH |= (0x0B << 4);  // PA9 TX
    GPIOA->CRH |= (0x04 << 8);  // PA10 RX
    USART1->BRR = 0x271;        // Baudrate 115200 @ 72MHz
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

int fputc(int ch, FILE *f) {
    while (!(USART1->SR & USART_SR_TXE));
    USART1->DR = (ch & 0xFF);
    return ch;
}

// ==========================================================
// BIẾN TOÀN CỤC (GLOBAL VARIABLES)
// ==========================================================
volatile unsigned int global_tick = 0; 
volatile uint8_t xx = 0x00;           
volatile uint8_t xx_mpu_state = 0x00; 
volatile uint8_t xx_mpu_id = 0x00;
uint32_t last_csv_tick = 0;
uint32_t last_print_tick = 0;

// --- BIẾN ĐO LƯỜNG GIAO TIẾP NGOẠI VI (I2C MPU6050) ---
uint32_t total_i2c_bytes = 0;       // Tổng lưu lượng data đã kéo từ cảm biến (Bytes)
uint32_t i2c_bytes_in_last_sec = 0; // Biến tạm đếm số byte ngoại vi trong 1 giây qua
uint32_t i2c_speed_bps = 0;         // Tốc độ truyền ngoại vi (Bytes/sec)
uint32_t last_i2c_speed_tick = 0;   // Mốc thời gian để chốt sổ tính tốc độ I2C

void SysTick_Handler(void) { global_tick++; }

int main(void) {
    // 1. Khởi tạo GPIO Nút bấm
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    GPIOA->CRL &= ~(0xFF << 0);            
    GPIOA->CRL |= (0x88 << 0);             
    GPIOA->ODR |= (1UL << 0) | (1UL << 1); 

    // 2. Khởi tạo Ngoại vi cốt lõi
    I2C1_Init();
    SysTick_Config(SystemCoreClock / 1000); 
    HAL_TIM4_Micros_Init(); 
    UART1_Debug_Init();     
    
    // 3. Khởi tạo Tay chân (PWM Motor) và Não bộ (PID)
    HAL_TIM3_PWM_Init(); // Bật cấp điện giả lập cho 4 Motor
    PID_Init();          // Khởi tạo các hệ số Kp, Ki, Kd
    
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
                    
                    // In Header của file CSV (Cập nhật 2 cột I2C)
                    printf("Time(ms),Roll,Pitch,Yaw,M1,M2,M3,M4,I2C_Speed(Bps),I2C_Volume(Bytes)\n"); 
                    
                    // Reset bộ đếm ngoại vi I2C mỗi khi cất cánh
                    total_i2c_bytes = 0;
                    i2c_bytes_in_last_sec = 0;
                    i2c_speed_bps = 0;
                    last_i2c_speed_tick = global_tick;
                } else {
                    xx = 0x00; 
                    printf("LOI: Day I2C bi long, khong the Calibrate!\n");
                }
            } 
            else {
                printf("\nDA TAT HE THONG (0x04)!\n");
                MPU_Sleep();       
                memset((void*)&Drone_IMU, 0, sizeof(MPU_Motion_t));
                
                // Tắt ngóm 4 động cơ cho an toàn!
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
                // Ép ga về 0 khi dừng đọc
                HAL_TIM3_PWM_SetDuty(1, 0); HAL_TIM3_PWM_SetDuty(2, 0);
                HAL_TIM3_PWM_SetDuty(3, 0); HAL_TIM3_PWM_SetDuty(4, 0);
            }
            btn_PA0.event_code = BTN_EVENT_NONE; 
        }

        switch (xx) {
            case 0x02:
                // 1. Hút Data và Lọc Góc
                // MPU_Fusion_Read_Burst() kéo đúng 14 Bytes từ I2C mỗi lần gọi
                if (MPU_Fusion_Read_Burst() == 1) {
                    // CỘNG DỒN LƯU LƯỢNG I2C (14 Bytes cho 1 gói Gia tốc + Gyro + Nhiệt độ)
                    total_i2c_bytes += 14;
                    i2c_bytes_in_last_sec += 14;

                    MPU_Fusion_Compute();
                    
                    // 2. CHẠY THUẬT TOÁN PID
                    //PID_Compute(Drone_IMU.Roll, Drone_IMU.Pitch, Drone_IMU.Yaw, Drone_IMU.dt); // Code CŨ đang bị lệch trục
                    //PID_Compute(Drone_IMU.Pitch, Drone_IMU.Roll, Drone_IMU.Yaw, Drone_IMU.dt); // Code MỚI (Hack xoay mạch 90 độ)
                    PID_Compute(Drone_IMU.Pitch, -Drone_IMU.Roll, Drone_IMU.Yaw, Drone_IMU.dt); // Thêm dấu TRỪ (-) trước Roll, fix Flip of Death
                    
                    // 3. ĐẨY XUỐNG BỘ TRỘN ĐỘNG CƠ 
                    Motor_Mixer(400); 
                    
                    // --- ĐO LƯỜNG TỐC ĐỘ NGOẠI VI I2C (Mỗi 1 giây chốt sổ 1 lần) ---
                    if (global_tick - last_i2c_speed_tick >= 1000) {
                        i2c_speed_bps = i2c_bytes_in_last_sec; // Chốt tốc độ Bytes/sec
                        i2c_bytes_in_last_sec = 0;             // Reset đếm lại cho giây tiếp theo
                        last_i2c_speed_tick = global_tick;
                    }

                    // 4. XUẤT TELEMETRY CSV LÊN MÁY TÍNH (5Hz = 200ms/lần)
                    if (global_tick - last_csv_tick >= 200) { 
                        printf("%u,%.2f,%.2f,%.2f,%d,%d,%d,%d,%lu,%lu\n", 
                               global_tick,
                               Drone_IMU.Roll, Drone_IMU.Pitch, Drone_IMU.Yaw,
                               TIM3->CCR1, TIM3->CCR2, TIM3->CCR3, TIM3->CCR4,
                               i2c_speed_bps, total_i2c_bytes);
                        last_csv_tick = global_tick;
                    }
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