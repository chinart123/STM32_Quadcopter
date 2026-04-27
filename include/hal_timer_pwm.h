#ifndef HAL_TIMER_PWM_H
#define HAL_TIMER_PWM_H

#include "stm32f10x.h"

// Hàm khởi tạo TIM3 xuất PWM 4kHz cho 4 Motor Coreless 8520
// Kênh 1: PA6 | Kênh 2: PA7 | Kênh 3: PB0 | Kênh 4: PB1
void HAL_TIM3_PWM_Init(void);

// Hàm thay đổi tốc độ Motor (Duty Cycle từ 0 đến 999)
// channel: 1, 2, 3, 4 tương ứng với 4 góc của Quadcopter
void HAL_TIM3_PWM_SetDuty(uint8_t channel, uint16_t duty);

// =================================================================
// --- THÊM MỚI Ở STAGE 7: HÀM ĐO THỜI GIAN MICRO-GIÂY (DÙNG TIM4) ---
// =================================================================
void HAL_TIM4_Micros_Init(void);
uint16_t micros(void);
#endif