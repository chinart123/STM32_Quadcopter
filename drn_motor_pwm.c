#include "drn_motor_pwm.h"
#include "drn_main_board_choose.h"

#ifdef STM32F103C8T6
    #include "stm32f10x.h"
#elif defined(ESP32_S3_SUPERMINI)
    // #include "driver/ledc.h"
#endif

// =========================================================
// CẤP PHÁT BỘ NHỚ CHO CÁC BIẾN TEST (Không dùng static nữa)
// =========================================================
float xx_pwm_motor_1 = 0.0f; 
float xx_pwm_step    = 5.0f; 
uint8_t xx_gate_state = CMD_GATE_CLOSE; 
uint8_t xx_ramp_state = CMD_RAMP_100_TO_0; 

// Biến này chỉ dùng đếm thời gian nội bộ nên vẫn giấu đi
static uint32_t last_motor_update_tick = 0;

void DRN_Motor_PWM_Init(void) {
#ifdef STM32F103C8T6
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;

    GPIOA->CRL &= ~((0xF << 24) | (0xF << 28)); 
    GPIOA->CRL |=  ((0xB << 24) | (0xB << 28)); 
    GPIOB->CRL &= ~((0xF << 0)  | (0xF << 4)); 
    GPIOB->CRL |=  ((0xB << 0)  | (0xB << 4)); 

    // Tần số 500Hz (Clock 72MHz)
    TIM3->PSC = 17;     
    TIM3->ARR = 999;   
    TIM3->EGR |= TIM_EGR_UG;              
    TIM3->SR &= ~TIM_SR_UIF;

    TIM3->CCMR1 &= ~((0xFF << 0) | (0xFF << 8)); 
    TIM3->CCMR1 |= (6 << 4) | (6 << 12) | (1 << 3) | (1 << 11);         
    TIM3->CCMR2 &= ~((0xFF << 0) | (0xFF << 8)); 
    TIM3->CCMR2 |= (6 << 4) | (6 << 12) | (1 << 3) | (1 << 11);         

    TIM3->CCER &= ~((0xF << 0) | (0xF << 4) | (0xF << 8) | (0xF << 12));
    TIM3->CCER |= (1 << 0) | (1 << 4) | (1 << 8) | (1 << 12); 

    TIM3->CCR1 = 0; TIM3->CCR2 = 0; TIM3->CCR3 = 0; TIM3->CCR4 = 0;
    TIM3->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN;
#elif defined(ESP32_S3_SUPERMINI)
    // Code cấu hình LEDC PWM cho ESP32...
#endif
}

void DRN_Motor_Update_Logic(DRN_ButtonEvent_t arm_event, DRN_ButtonEvent_t mode_event) {
    if (arm_event == BTN_EVENT_SINGLE_CLICK) {
        if (xx_gate_state == CMD_GATE_CLOSE) xx_gate_state = CMD_GATE_OPEN;  
        else xx_gate_state = CMD_GATE_CLOSE; 
    }

    if (mode_event == BTN_EVENT_SINGLE_CLICK) {
        if (xx_ramp_state == CMD_RAMP_100_TO_0) xx_ramp_state = CMD_RAMP_0_TO_50;        
        else if (xx_ramp_state == CMD_RAMP_0_TO_50) xx_ramp_state = CMD_RAMP_50_TO_100;      
        else if (xx_ramp_state == CMD_RAMP_50_TO_100) xx_ramp_state = CMD_RAMP_100_TO_0;       
    }
}

static void set_duty_channel_1(float percent) {
    if (percent > 100.0f) percent = 100.0f;
    if (percent < 0.0f)  percent = 0.0f;
    
#ifdef STM32F103C8T6
    uint16_t ccr_value = (uint16_t)((percent / 100.0f) * 999.0f);
    TIM3->CCR2 = ccr_value;
#elif defined(ESP32_S3_SUPERMINI)
    // Code ESP32 đẩy duty...
#endif
}

void DRN_Motor_Run_Task(uint32_t current_time) {
    if (current_time - last_motor_update_tick >= 100) {
        last_motor_update_tick = current_time;

        if (xx_gate_state == CMD_GATE_CLOSE) { 
            xx_pwm_motor_1 = 0.0f;
            set_duty_channel_1(0.0f);
        }
        else if (xx_gate_state == CMD_GATE_OPEN) { 
            if (xx_ramp_state == CMD_RAMP_0_TO_50) {
                xx_pwm_motor_1 += xx_pwm_step;
                if (xx_pwm_motor_1 > 50.0f) xx_pwm_motor_1 = 50.0f; 
            }
            else if (xx_ramp_state == CMD_RAMP_50_TO_100) {
                xx_pwm_motor_1 += xx_pwm_step;
                if (xx_pwm_motor_1 > 100.0f) xx_pwm_motor_1 = 100.0f;
            }
            else if (xx_ramp_state == CMD_RAMP_100_TO_0) {
                xx_pwm_motor_1 -= xx_pwm_step;
                if (xx_pwm_motor_1 < 0.0f) xx_pwm_motor_1 = 0.0f;
            }
            set_duty_channel_1(xx_pwm_motor_1);
        }
    }
}