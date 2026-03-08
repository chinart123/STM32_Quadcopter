#include "stm32f10x.h"
#include "button.h"
#include "i2c_mpu_debug.h" 
#include "xx_mpu_data_fusion.h" // Nhúng não bộ Toán học
#include "hal_timer_pwm.h"      // <--- BẮT BUỘC PHẢI CÓ ĐỂ GỌI ĐƯỢC TIM4
#include <string.h>
#include <stdio.h>              // Dùng cho printf

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

// Ghi đè fputc để printf đẩy dữ liệu ra dây PA9
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
volatile uint8_t xx_mpu_state = 0x00; // Trạng thái đường truyền I2C
volatile uint8_t xx_mpu_id = 0x00;    // Mã ID của cảm biến

void SysTick_Handler(void) { global_tick++; }

int main(void) {
    // 1. Khởi tạo xung nhịp và GPIO cho 2 nút bấm PA0, PA1
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    GPIOA->CRL &= ~(0xFF << 0);            
    GPIOA->CRL |= (0x88 << 0);             
    GPIOA->ODR |= (1UL << 0) | (1UL << 1); 

    // 2. Khởi tạo ngoại vi
    I2C1_Init();
    SysTick_Config(SystemCoreClock / 1000); 
    
    // 3. Khởi tạo các module của Stage 7
    HAL_TIM4_Micros_Init(); // BẬT CỖ MÁY THỜI GIAN ĐỂ TÍNH dt VÀ YAW
    UART1_Debug_Init();     // BẬT CỔNG SERIAL ĐỂ PRINTF
    
    printf("\n--- HET THONG QUADCOPTER SAN SANG ---\n");

    // ==========================================================
    // TRẠNG THÁI BAN ĐẦU (0x00)
    // ==========================================================
    xx = 0x00;
    xx_mpu_state = 0x00;
    xx_mpu_id = 0x00;
    MPU_Sleep(); 

    // Các biến dùng cho Radar đo tốc độ
    uint32_t byte_counter = 0;
    uint32_t last_print_tick = 0;

    while (1) {
        // Nuôi sống bộ Quét nút bấm chống dội (Debounce)
        button_state_hardware_scan();
        button_fsm_process(&btn_PA0, global_tick);
        button_fsm_process(&btn_PA1, global_tick);

        // ==========================================================
        // VÒNG LẶP CHA: QUYỀN SINH SÁT (NÚT PA1)
        // ==========================================================
        if (btn_PA1.event_code == BTN_EVENT_SINGLE_CLICK) {
            if (xx == 0x00 || xx == 0x04) {
                printf("\nKhoi dong He thong...\n");
                MPU_WakeUp();
                MPU_Fusion_Init(); 
                MPU_Read_WhoAmI(&xx_mpu_state, &xx_mpu_id); 
                
                printf("Dang Calibrate, vui long de im mach...\n");
                
                // Kỷ luật sắt! Chỉ chuyển trạng thái 0x01 khi Calibrate an toàn tuyệt đối
                if (MPU_Fusion_Calibrate() == 1) {
                    xx = 0x01; 
                    printf("Calibrate XONG! San sang xa lu (0x02)\n");
                } else {
                    xx = 0x00; // Đứt dây I2C -> Ép nằm im tại 0x00!
                    printf("LOI: Day I2C bi long, khong the Calibrate!\n");
                }
            } 
            else {
                printf("\nDA TAT HE THONG (0x04)!\n");
                MPU_Sleep();       
                memset((void*)&Drone_IMU, 0, sizeof(MPU_Motion_t));
                xx_mpu_id = 0x00;
                xx_mpu_state = 0x00; 
                xx = 0x04;         
            }
            btn_PA1.event_code = BTN_EVENT_NONE; 
        }

        // ==========================================================
        // VÒNG LẶP CON: CHỨC NĂNG ĐỌC I2C (NÚT PA0)
        // ==========================================================
        if (btn_PA0.event_code == BTN_EVENT_SINGLE_CLICK) {
            if (xx == 0x01 || xx == 0x03) {
                xx = 0x02; // Chuyển sang THU THẬP LIÊN TỤC
                printf("\n--- BAT DAU XA LU DATA ---\n");
            } 
            else if (xx == 0x02) {
                xx = 0x03; // Chuyển sang DỪNG ĐỌC
            }
            btn_PA0.event_code = BTN_EVENT_NONE; 
        }

        // ==========================================================
        // THỰC THI CHỨC NĂNG THEO TRẠNG THÁI
        // ==========================================================
        switch (xx) {
            case 0x02:
                // Bọc bảo vệ kép! Chỉ tính Lượng giác khi hút Data thành công!
                if (MPU_Fusion_Read_Burst() == 1) {
                    MPU_Fusion_Compute();
                    
                    // --- ĐO LƯU LƯỢNG VÀ PRINTF (Mỗi giây 1 lần) ---
                    byte_counter += 14; 
                    if (global_tick - last_print_tick >= 1000) {
                        printf("Speed: %4d Bytes/s | R: %6.2f | P: %6.2f | Y: %6.2f\n", 
                               byte_counter, 
                               Drone_IMU.Roll, 
                               Drone_IMU.Pitch, 
                               Drone_IMU.Yaw);
                               
                        byte_counter = 0; // Reset đếm lại cho giây tiếp theo
                        last_print_tick = global_tick;
                    }
                }
                break;
                
            case 0x03:
                // Vẫn In ra màn hình nhưng số liệu bị đóng băng (không chạy Compute)
                if (global_tick - last_print_tick >= 1000) {
                    printf("DANG DUNG (0x03)   | R: %6.2f | P: %6.2f | Y: %6.2f\n", 
                           Drone_IMU.Roll, Drone_IMU.Pitch, Drone_IMU.Yaw);
                    last_print_tick = global_tick;
                }
                break;

            default:
                break;
        }
    }
}