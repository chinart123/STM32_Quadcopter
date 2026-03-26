#ifndef MAIN_BOARD_CHOOSE_H
#define MAIN_BOARD_CHOOSE_H

// ==========================================
// 1. CHỌN BOARD ĐỂ BIÊN DỊCH
// ==========================================
#define STM32F103C8T6 
// #define ESP32_S3_SUPERMINI 

// ==========================================
// 2. GOD HEADER: NHÚNG TOÀN BỘ TỪ ĐIỂN VÀO ĐÂY
// ==========================================
#include "drn_timer_pwm.h"
#include "drn_button.h"
// #include "drn_i2c.h" 

// ==========================================
// 3. CÁC HÀM TỔNG QUẢN
// ==========================================
void main_board_choose_Init(void);
void main_board_choose_Delay_ms(unsigned int ms);

#endif