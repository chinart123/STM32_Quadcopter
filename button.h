#ifndef BUTTON_H
#define BUTTON_H

#include "stm32f10x.h"

// 1. Nhóm cấu hình thời gian (Dùng chung cho TẤT CẢ các nút)
typedef enum {
    BTN_TICK_DEBOUNCE   = 3,     // 3 nhịp = 30ms (Lọc nhiễu)
    BTN_TICK_DOUBLE_GAP = 30     // 30 nhịp = 300ms (Chờ click lần 2)
    // ĐÃ XÓA BTN_TICK_HOLD Ở ĐÂY VÌ MỖI NÚT SẼ CÓ THỜI GIAN HOLD RIÊNG!
} ButtonTiming_t;

// 2. Nhóm sự kiện đầu ra
typedef enum {
    BTN_EVENT_NONE = 0,
    BTN_EVENT_SINGLE_CLICK,
    BTN_EVENT_DOUBLE_CLICK,
    BTN_EVENT_HOLD
} ButtonEvent_TypeDef;

// 3. Hồ sơ bệnh án của Nút bấm
typedef struct {
    unsigned char       pin_state;        // Trạng thái vật lý
    unsigned int        press_duration;   // Đếm thời gian đè
    unsigned int        release_duration; // Đếm thời gian nhả
    unsigned char       click_count;      // Số lần click
    ButtonEvent_TypeDef event_code;       // Lời chẩn đoán cuối cùng
    unsigned int        last_update_tick; // Mốc thời gian SysTick
    
    // THAM SỐ ĐỘNG: Thời gian Hold mục tiêu của riêng nút này
    unsigned int        hold_target;      // Ví dụ: 500 (5s) hoặc 200 (2s)
} Button_Context;

// Khai báo 2 nút để main.c có thể soi hồ sơ
extern Button_Context btn_PA0;
extern Button_Context btn_PA1;

void button_state_hardware_scan(void);
void button_fsm_process(Button_Context *btn, unsigned int current_tick);

#endif // BUTTON_H