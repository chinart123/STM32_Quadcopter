#include "stm32f10x.h"
#include "button.h"
#include "i2c_mpu_debug.h" 

// Biến toàn cục để Debug
volatile unsigned int global_tick = 0; 
volatile uint8_t xx = 0x00;           
volatile uint8_t xx_mpu_state = 0x00; 
volatile uint8_t xx_mpu_id = 0x00;    

void SysTick_Handler(void) { global_tick++; }

int main(void) {
    // 1. Khởi tạo xung nhịp và GPIO cho 2 nút bấm
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    GPIOA->CRL &= ~(0xFF << 0);            
    GPIOA->CRL |= (0x88 << 0);             
    GPIOA->ODR |= (1UL << 0) | (1UL << 1); 

    // 2. Khởi tạo ngoại vi
    I2C1_Init();
    SysTick_Config(SystemCoreClock / 1000); 

    // ==========================================================
    // QUY TRÌNH 1: RESET (Mọi thứ về 0, ép MPU ngủ sâu)
    // ==========================================================
    xx = 0x00;
    xx_mpu_state = 0x00;
    xx_mpu_id = 0x00;
    MPU_Sleep(); // Khóa không cho MPU kết nối tín hiệu

    while (1) {
        // Cấp xung nhịp FSM Nút bấm
        button_state_hardware_scan();
        button_fsm_process(&btn_PA0, global_tick);
        button_fsm_process(&btn_PA1, global_tick);

        // ==========================================================
        // QUY TRÌNH 2 & 4: NÚT PA1 (Công tắc Nguồn MPU)
        // ==========================================================
        if (btn_PA1.event_code == BTN_EVENT_SINGLE_CLICK) {
            
            // Nếu đang tắt (0x00) hoặc đã thoát (0x04) -> Cấp nguồn
            if (xx == 0x00 || xx == 0x04) {
                MPU_WakeUp();
                xx = 0x01; 
                // Không cập nhật id và state ở bước này
            } 
            // Nếu đang trong luồng bật (0x01, 0x02, 0x03) -> Tắt nguồn
            else {
                MPU_Sleep();
                xx = 0x04;
                xx_mpu_id = 0x00;
                xx_mpu_state = 0x00; // Reset về trạng thái trắng
            }
            btn_PA1.event_code = BTN_EVENT_NONE; 
        }

        // ==========================================================
        // QUY TRÌNH 2 & 3: NÚT PA0 (Thu thập & Xóa dữ liệu)
        // ==========================================================
        if (btn_PA0.event_code == BTN_EVENT_SINGLE_CLICK) {
            
            // Chỉ cho phép hoạt động nếu MPU đang được bật (xx=0x01 hoặc 0x03)
            if (xx == 0x01 || xx == 0x03) {
                xx = 0x02;
                MPU_Read_WhoAmI(&xx_mpu_state, &xx_mpu_id); // Cập nhật danh tính và mã lỗi
            } 
            // Bấm lần 2 khi đang có data -> Xóa data
            else if (xx == 0x02) {
                xx = 0x03;
                xx_mpu_id = 0x00;
                xx_mpu_state = 0x00;
            }
            btn_PA0.event_code = BTN_EVENT_NONE; 
        }
    }
}