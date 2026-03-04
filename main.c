#include "stm32f10x.h"
#include "hal_timer_pwm.h"

int main(){

		HAL_TIM3_PWM_Init();
    HAL_TIM3_PWM_SetDuty(1, 100); // M1 chạy 10% PA6
    HAL_TIM3_PWM_SetDuty(2, 400); // M2 chạy 40% PA7
    HAL_TIM3_PWM_SetDuty(3, 700); // M3 chạy 70% PB0
    HAL_TIM3_PWM_SetDuty(4, 999); // M4 max ga 100% PB1
		
    while(1)
    {
						
    }
}