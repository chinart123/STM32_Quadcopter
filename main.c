#include "main_board_choose.h" 

// =========================================================
// ĐỊNH NGHĨA CÁC TRẠNG THÁI LOGIC CỦA HỆ THỐNG
// =========================================================
#define CMD_GATE_OPEN       0x01  // Mở cổng (Arm)
#define CMD_GATE_CLOSE      0x04  // Đóng cổng (Disarm)

// 3 Trạng thái ga mới
#define CMD_RAMP_0_TO_50    0x02  // Tăng ga từ 0 lên 50%
#define CMD_RAMP_50_TO_100  0x05  // Tăng ga từ 50 lên 100%
#define CMD_RAMP_100_TO_0   0x03  // Giảm ga từ 100 về 0%

// =========================================================
// CÁC BIẾN TEST VÀ LƯU TRẠNG THÁI
// =========================================================
float xx_pwm_motor_1 = 0.0f; 
float xx_pwm_step    = 5.0f; // Mỗi nhịp cộng/trừ 5% (Mất 1 giây để tăng 50%)

// Trạng thái khởi động an toàn
uint8_t xx_gate_state = CMD_GATE_CLOSE; 
uint8_t xx_ramp_state = CMD_RAMP_100_TO_0; // Khởi đầu ở trạng thái giảm về 0 

// Biến lưu trạng thái vật lý của nút (để bắt sườn - Edge Detection)
uint8_t xx_last_pa1 = 0;
uint8_t xx_last_pa0 = 0;
// Thêm dòng này để dỗ dành file i2c_mpu_debug.c
int xx_mpu_state = 0;
int main(void) {
    // Khởi tạo toàn bộ hệ thống phần cứng
    main_board_choose_Init();

    while(1) {
        // ---------------------------------------------------------
        // BƯỚC 1: QUÉT NÚT NHẤN VÀ CẬP NHẬT TRẠNG THÁI (FSM)
        // ---------------------------------------------------------
        
        uint8_t current_pa1 = DRN_Button_Read_PA1(); 
        uint8_t current_pa0 = DRN_Button_Read_PA0(); 

        // Xử lý PA1 (Công tắc Cổng: Đóng <-> Mở)
        if (current_pa1 == 1 && xx_last_pa1 == 0) {
            if (xx_gate_state == CMD_GATE_CLOSE) {
                xx_gate_state = CMD_GATE_OPEN;  
            } else {
                xx_gate_state = CMD_GATE_CLOSE; 
            }
        }
        xx_last_pa1 = current_pa1; 

        // Xử lý PA0 (Công tắc Ga: Vòng lặp 3 bước)
        if (current_pa0 == 1 && xx_last_pa0 == 0) {
            if (xx_ramp_state == CMD_RAMP_100_TO_0) {
                xx_ramp_state = CMD_RAMP_0_TO_50;        // Nhịp 1: 0 -> 50
            } 
            else if (xx_ramp_state == CMD_RAMP_0_TO_50) {
                xx_ramp_state = CMD_RAMP_50_TO_100;      // Nhịp 2: 50 -> 100
            } 
            else if (xx_ramp_state == CMD_RAMP_50_TO_100) {
                xx_ramp_state = CMD_RAMP_100_TO_0;       // Nhịp 3: 100 -> 0
            }
        }
        xx_last_pa0 = current_pa0; 


        // ---------------------------------------------------------
        // BƯỚC 2: BƠM GA XUỐNG ĐỘNG CƠ DỰA TRÊN TRẠNG THÁI
        // ---------------------------------------------------------
        
        if (xx_gate_state == CMD_GATE_CLOSE) { 
            // KHÓA AN TOÀN: Ép ga về 0 lập tức nếu cổng đóng
            xx_pwm_motor_1 = 0.0f;
            DRN_Timer_PWM_SetDuty(1, 0.0f);
        }
        else if (xx_gate_state == CMD_GATE_OPEN) { 
            
            // LOGIC 1: Đẩy lên 50%
            if (xx_ramp_state == CMD_RAMP_0_TO_50) {
                xx_pwm_motor_1 += xx_pwm_step;
                // Nếu lỡ đang ở 100% mà bấm nhầm sang chế độ này, nó sẽ giật thẳng về 50% (An toàn)
                if (xx_pwm_motor_1 > 50.0f) {
                    xx_pwm_motor_1 = 50.0f; 
                }
            }
            // LOGIC 2: Đẩy lên 100%
            else if (xx_ramp_state == CMD_RAMP_50_TO_100) {
                xx_pwm_motor_1 += xx_pwm_step;
                if (xx_pwm_motor_1 > 100.0f) {
                    xx_pwm_motor_1 = 100.0f;
                }
            }
            // LOGIC 3: Hạ cánh về 0%
            else if (xx_ramp_state == CMD_RAMP_100_TO_0) {
                xx_pwm_motor_1 -= xx_pwm_step;
                if (xx_pwm_motor_1 < 0.0f) {
                    xx_pwm_motor_1 = 0.0f;
                }
            }

            // Xuất tín hiệu ra chân vật lý PA6
            DRN_Timer_PWM_SetDuty(1, xx_pwm_motor_1);
        }

        // ---------------------------------------------------------
        // Tốc độ quét 100ms
        // Mất đúng 1 giây để leo từ 0->50%, và 1 giây nữa để lên 100%
        // ---------------------------------------------------------
        main_board_choose_Delay_ms(100); 
    }
}