#include "stm32f10x.h"
#include "button.h"
#include "i2c_mpu_debug.h" 
#include "xx_mpu_data_fusion.h" // Nhúng não bộ Toán học vào đây
#include <string.h>
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

    // 2. Khởi tạo ngoại vi I2C và SysTick Timer
    I2C1_Init();
    SysTick_Config(SystemCoreClock / 1000); 

    // ==========================================================
    // TRẠNG THÁI BAN ĐẦU (0x00): Tắt nguồn, mọi thứ ngủ yên
    // ==========================================================
    xx = 0x00;
    xx_mpu_state = 0x00;
    xx_mpu_id = 0x00;
    MPU_Sleep(); // Khóa không cho MPU kết nối tín hiệu

    while (1) {
        // Nuôi sống bộ Quét nút bấm chống dội (Debounce)
        button_state_hardware_scan();
        button_fsm_process(&btn_PA0, global_tick);
        button_fsm_process(&btn_PA1, global_tick);

        // ==========================================================
        // KHỐI 1: XỬ LÝ CHUYỂN TRẠNG THÁI (PHÂN CẤP CHA - CON)
        // ==========================================================
        
        // --- NÚT PA1 (CÔNG TẮC CHA): QUYỀN SINH SÁT (0x01 và 0x04) ---
        if (btn_PA1.event_code == BTN_EVENT_SINGLE_CLICK) {
            // Nếu đang tắt (0x00) hoặc đang ngủ (0x04) -> BẬT
            if (xx == 0x00 || xx == 0x04) {
                MPU_WakeUp();
                MPU_Fusion_Init(); // Nạp cấu hình bay (±8g, ±500dps, DLPF)
                MPU_Read_WhoAmI(&xx_mpu_state, &xx_mpu_id); // GỌI TÀI XẾ CẬP NHẬT TRẠNG THÁI I2C TẠI ĐÂY
                // ==========================================================
                // --- THÊM MỚI Ở STAGE 6: Đọc 1000 mẫu để khử sai số ---
                // Sếp chú ý: Lúc bấm nút này mạch phải đang nằm trên bàn!
                // ==========================================================
                MPU_Fusion_Calibrate();
                xx = 0x01;         // CHA BẬT: Hệ thống sẵn sàng (Standby)
            } 
            // Nếu hệ thống đang bật (0x01, 0x02, 0x03) -> TẮT
            else {
                MPU_Sleep();       // Ép MPU ngủ đông
                // Dọn sạch sành sanh không chừa một mảnh rác nào trong Struct
                memset((void*)&Drone_IMU, 0, sizeof(MPU_Motion_t));
                // Đóng băng hiện trường: Chỉ xóa góc bay, giữ nguyên Data thô
//                Drone_IMU.Roll = 0.0f; 
//                Drone_IMU.Pitch = 0.0f; 
//                Drone_IMU.Yaw = 0.0f;
                
                xx = 0x04;         // CHA TẮT: Hệ thống đi ngủ
            }
            btn_PA1.event_code = BTN_EVENT_NONE; 
        }

        // --- NÚT PA0 (CÔNG TẮC CON): CHỨC NĂNG (0x02 và 0x03) ---
        if (btn_PA0.event_code == BTN_EVENT_SINGLE_CLICK) {
            // Thằng Con CHỈ được phép nhận lệnh khi Cha đang BẬT (Khác 0x00 và 0x04)
            if (xx == 0x01 || xx == 0x03) {
                // Từ trạng thái chờ hoặc đang dừng -> Chuyển sang THU THẬP LIÊN TỤC
                xx = 0x02; 
            } 
            else if (xx == 0x02) {
                // Đang thu thập liên tục -> Chuyển sang DỪNG ĐỌC
                // Đóng băng hiện trường: Chỉ xóa góc bay, giữ nguyên Data thô cuối cùng
//                Drone_IMU.Roll = 0.0f;  // nếu muốn xóa góc
//                Drone_IMU.Pitch = 0.0f; // nếu muốn xóa góc
//                Drone_IMU.Yaw = 0.0f;   // nếu muốn xóa góc
                
                xx = 0x03; 
            }
            btn_PA0.event_code = BTN_EVENT_NONE; 
        }

        // ==========================================================
        // KHỐI 2: THỰC THI LIÊN TỤC THEO TRẠNG THÁI
        // ==========================================================
        switch (xx) {
            case 0x02:
                // [CHẾ ĐỘ 0x02 - THU THẬP & TÍNH TOÁN LIÊN TỤC] 
                // Cỗ máy toán học chạy hết công suất ở đây (Xả lũ)
                MPU_Fusion_Read_Burst();
                MPU_Fusion_Compute();
                break;
                
            // Các trạng thái 0x00, 0x01, 0x03, 0x04 chỉ đứng yên giữ trạng thái
            default:
                break;
        }
    }
}