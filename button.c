#include "button.h"

// =========================================================
// KHỞI TẠO HỒ SƠ (Chỉ khai báo ĐÚNG 1 LẦN duy nhất ở đây)
// =========================================================
// Cấu trúc: {pin_state, press_duration, release_duration, click_count, event_code, last_update_tick, HOLD_TARGET}

// Nút PA0: Yêu cầu Hold 5 giây (500 nhịp)
Button_Context btn_PA0 = {1, 0, 0, 0, BTN_EVENT_NONE, 0, 500}; 

// Nút PA1: Yêu cầu Hold 2 giây (200 nhịp)
Button_Context btn_PA1 = {1, 0, 0, 0, BTN_EVENT_NONE, 0, 200}; 


// =========================================================
// HÀM 1: ĐỌC VẬT LÝ (HARDWARE LAYER)
// =========================================================
void button_state_hardware_scan(void) {
    btn_PA0.pin_state = (GPIOA->IDR & (1UL << 0)) ? 1 : 0;
    btn_PA1.pin_state = (GPIOA->IDR & (1UL << 1)) ? 1 : 0;
}

// =========================================================
// HÀM 2: MÁY TRẠNG THÁI FSM (MIDDLEWARE LAYER)
// =========================================================
void button_fsm_process(Button_Context *btn, unsigned int current_tick) {
    
    // Chạy logic mỗi 10ms (Pacing)
    if (current_tick - btn->last_update_tick >= 10) {
        
        btn->last_update_tick = current_tick; 
        
        // ---------------------------------------------------
        // TRẠNG THÁI 1: NÚT ĐANG BỊ ĐÈ (pin_state == 0)
        // ---------------------------------------------------
        if (btn->pin_state == 0) { 
            btn->release_duration = 0;
            if (btn->press_duration < 60000) btn->press_duration++;

            // [LÕI ĐỘNG]: Kiểm tra thời gian Hold theo đúng thông số của từng nút!
            if (btn->press_duration == btn->hold_target) { 
                btn->event_code = BTN_EVENT_HOLD;
                btn->click_count = 0;
            }
        } 
        // ---------------------------------------------------
        // TRẠNG THÁI 2: NÚT ĐANG ĐƯỢC NHẢ RA (pin_state == 1)
        // ---------------------------------------------------
        else { 
            // Bắt khoảnh khắc vừa nhả tay
            if (btn->press_duration > 0) { 
                
                // [LÕI ĐỘNG]: Lọc nhiễu > 30ms VÀ Đè chưa tới mức Hold -> Tính là Click
                if (btn->press_duration > BTN_TICK_DEBOUNCE && btn->press_duration < btn->hold_target) {
                    btn->click_count++;
                }
                btn->press_duration = 0; // Đặt lại bộ đếm đè
            }

            // Đang trong thời gian chờ (gap time) để xem có click nhịp 2 không
            if (btn->click_count > 0) {
                if (btn->release_duration < 60000) btn->release_duration++;

                // Đủ 2 click là chốt luôn Double Click
                if (btn->click_count == 2) {
                    btn->event_code = BTN_EVENT_DOUBLE_CLICK;
                    btn->click_count = 0;
                    btn->release_duration = 0;
                } 
                // Chờ quá lâu (>300ms) thì chốt Single Click
                else if (btn->release_duration > BTN_TICK_DOUBLE_GAP) { 
                    if (btn->click_count == 1) {
                        btn->event_code = BTN_EVENT_SINGLE_CLICK;
                    }
                    btn->click_count = 0;
                    btn->release_duration = 0;
                }
            } else {
                btn->release_duration = 0; // Nghỉ ngơi hoàn toàn
            }
        }
    }
}