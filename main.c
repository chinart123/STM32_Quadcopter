#include "drn_main_board_choose.h" 
int xx_mpu_state;
int main(void) {
    // Khởi tạo toàn bộ hệ thống
    drn_main_board_choose_Init();

    // Vòng lặp siêu tốc độ (Non-blocking Loop)
    while(1) {
        // Lấy thời gian hiện tại từ SysTick / FreeRTOS
        uint32_t current_time = DRN_Millis();

        // 1. Quét chân phần cứng liên tục (Tốc độ tối đa)
        DRN_Button_State_Hardware_Scan();

        // 2. Chạy FSM (Nó tự giới hạn nhịp 10ms bên trong lõi)
        DRN_Button_FSM_Process(&drn_btn_PA0, current_time);
        DRN_Button_FSM_Process(&drn_btn_PA1, current_time);

        // 3. Đẩy sự kiện tìm được sang cho Motor (Nếu sự kiện là NONE, motor sẽ bỏ qua)
        DRN_Motor_Update_Logic(drn_btn_PA1.event_code, drn_btn_PA0.event_code);

        // Xóa cờ sự kiện sau khi đã sử dụng xong
        drn_btn_PA1.event_code = BTN_EVENT_NONE;
        drn_btn_PA0.event_code = BTN_EVENT_NONE;

        // 4. Bơm ga từ từ (Task này sẽ tự thức dậy bơm ga mỗi 100ms)
        DRN_Motor_Run_Task(current_time);
    }
}