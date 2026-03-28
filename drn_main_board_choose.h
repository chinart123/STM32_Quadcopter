#ifndef DRN_MAIN_BOARD_CHOOSE_H
#define DRN_MAIN_BOARD_CHOOSE_H

// ==========================================
// 1. CHỌN BOARD ĐỂ BIÊN DỊCH
// ==========================================
#define STM32F103C8T6 
// #define ESP32_S3_SUPERMINI 

// ==========================================
// 2. NHÚNG TOÀN BỘ TỪ ĐIỂN VÀO ĐÂY
// ==========================================
#include "drn_time.h"
#include "drn_motor_pwm.h"
#include "drn_button.h"

// ==========================================
// 3. CÁC HÀM TỔNG QUẢN
// ==========================================
void drn_main_board_choose_Init(void);
void drn_main_board_choose_Delay_ms(unsigned int ms);

#endif