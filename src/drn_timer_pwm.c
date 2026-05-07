#include "drn_timer_pwm.h"
#include "drn_main_board_choose.h"

#ifdef STM32F103C8T6
    #include "stm32f10x.h"
#elif defined(ESP32_S3_SUPERMINI)
    // #include "driver/ledc.h"
#endif

void DRN_Timer_PWM_Init(void) {
#ifdef STM32F103C8T6
    // Cấp Clock cho TIM3, Port A, Port B và AFIO
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;

    // Cấu hình PA6, PA7 (CH1, CH2) và PB0, PB1 (CH3, CH4) làm Alternate Function Push-Pull
    GPIOA->CRL &= ~((0xF << 24) | (0xF << 28)); 
    GPIOA->CRL |=  ((0xB << 24) | (0xB << 28)); 
    GPIOB->CRL &= ~((0xF << 0)  | (0xF << 4)); 
    GPIOB->CRL |=  ((0xB << 0)  | (0xB << 4)); 

    // Tần số 4kHz (Clock 72MHz) và Thước đếm 1000 bước
    TIM3->PSC = 17;     // Thay đổi bộ chia PSC từ 17 thành 143 để ép tần số về 500Hz, 72MHz / (143+1) = 500 
    TIM3->ARR = 999;   
    TIM3->EGR |= TIM_EGR_UG;              
    TIM3->SR &= ~TIM_SR_UIF;

    // Cấu hình PWM Mode 1
    TIM3->CCMR1 &= ~((0xFF << 0) | (0xFF << 8)); 
    TIM3->CCMR1 |= (6 << 4) | (6 << 12) | (1 << 3) | (1 << 11);         
    TIM3->CCMR2 &= ~((0xFF << 0) | (0xFF << 8)); 
    TIM3->CCMR2 |= (6 << 4) | (6 << 12) | (1 << 3) | (1 << 11);         

    // Bật Output 4 Kênh
    TIM3->CCER &= ~((0xF << 0) | (0xF << 4) | (0xF << 8) | (0xF << 12));
    TIM3->CCER |= (1 << 0) | (1 << 4) | (1 << 8) | (1 << 12); 

    TIM3->CCR1 = 0; TIM3->CCR2 = 0; TIM3->CCR3 = 0; TIM3->CCR4 = 0;
    TIM3->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN;
#elif defined(ESP32_S3_SUPERMINI)
    // Code ESP32 ...
#endif
}

void DRN_Timer_PWM_SetDuty(uint8_t channel_id, float percent) {
    if (percent > 100.0f) percent = 100.0f;
    if (percent < 0.0f)  percent = 0.0f;

#ifdef STM32F103C8T6
    uint16_t ccr_value = (uint16_t)((percent / 100.0f) * 999.0f);
    switch(channel_id) {
        case 1: TIM3->CCR1 = ccr_value; break;
        case 2: TIM3->CCR2 = ccr_value; break;
        case 3: TIM3->CCR3 = ccr_value; break;
        case 4: TIM3->CCR4 = ccr_value; break;
    }
#elif defined(ESP32_S3_SUPERMINI)
    // Code ESP32 ...
#endif
}