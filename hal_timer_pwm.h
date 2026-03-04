#ifndef HAL_TIMER_PWM_H
#define HAL_TIMER_PWM_H

#include "stm32f10x.h"

// Hàm khởi tạo TIM3 (Kênh 3: PB0, Kênh 4: PB1)
void HAL_TIM3_PWM_Init(void);

// Hàm thay đổi độ sáng (Duty Cycle từ 0 đến 999)
// Kênh 3 -> LED1, Kênh 4 -> LED2
void HAL_TIM3_PWM_SetDuty(uint8_t channel, uint16_t duty);

#endif