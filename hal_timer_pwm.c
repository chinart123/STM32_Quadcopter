#include "hal_timer_pwm.h"

void HAL_TIM3_PWM_Init(void) {
    // 1. Cấp xung nhịp cho TIM3, Port A, Port B và Alternate Function
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
		
    // 2. Cấu hình chân GPIO làm Alternate Function Push-Pull (50MHz)
    // Motor 1 (PA6) và Motor 2 (PA7)
    GPIOA->CRL &= ~((0xF << 24) | (0xF << 28)); // Xóa rác bit 24-31
    GPIOA->CRL |=  ((0xB << 24) | (0xB << 28)); // 0xB = 1011 (AF-PP)

    // Motor 3 (PB0) và Motor 4 (PB1)
    GPIOB->CRL &= ~((0xF << 0) | (0xF << 4));   // Xóa rác bit 0-7
    GPIOB->CRL |=  ((0xB << 0) | (0xB << 4));   // 0xB = 1011 (AF-PP)

    // 3. Cấu hình "Cây thước" 4kHz cho Động cơ 8520
    TIM3->PSC = 26;    // Tạm sửa thành 26 để lừa cái Simulator 108MHz
    TIM3->ARR = 999;   // 1000 vạch * 0.25us = 250us (Tương đương 4kHz)
    
    TIM3->EGR |= TIM_EGR_UG;              
    TIM3->SR &= ~TIM_SR_UIF;

    // 4. Cấu hình PWM Mode 1 cho CẢ 4 KÊNH
    // Kênh 1 & 2 (Thanh ghi CCMR1)
    TIM3->CCMR1 &= ~((0xFF << 0) | (0xFF << 8));
    TIM3->CCMR1 |= (6 << 4) | (6 << 12);         // OC1M, OC2M = 110
    TIM3->CCMR1 |= (1 << 3) | (1 << 11);         // OC1PE, OC2PE (Preload)

    // Kênh 3 & 4 (Thanh ghi CCMR2)
    TIM3->CCMR2 &= ~((0xFF << 0) | (0xFF << 8)); 
    TIM3->CCMR2 |= (6 << 4) | (6 << 12);         // OC3M, OC4M = 110
    TIM3->CCMR2 |= (1 << 3) | (1 << 11);         // OC3PE, OC4PE (Preload)

    // 5. Mở khóa xuất Output cho 4 kênh (CC1E, CC2E, CC3E, CC4E)
    TIM3->CCER &= ~((1 << 0) | (1 << 4) | (1 << 8) | (1 << 12)); // Xóa rác
    TIM3->CCER |=  ((1 << 0) | (1 << 4) | (1 << 8) | (1 << 12)); // Bật 4 công tắc

    // Khởi tạo tốc độ Motor mặc định = 0 (Đứng im)
    TIM3->CCR1 = 0; TIM3->CCR2 = 0;
    TIM3->CCR3 = 0; TIM3->CCR4 = 0;

    // 6. Cho phép đếm
    TIM3->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN;
}

void HAL_TIM3_PWM_SetDuty(uint8_t channel, uint16_t duty) {
    if (duty > 999) duty = 999; // Khóa an toàn, tránh vọt ga
    
    switch(channel) {
        case 1: TIM3->CCR1 = duty; break; // M1 - PA6
        case 2: TIM3->CCR2 = duty; break; // M2 - PA7
        case 3: TIM3->CCR3 = duty; break; // M3 - PB0
        case 4: TIM3->CCR4 = duty; break; // M4 - PB1
    }
}