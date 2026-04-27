#include "main_board_choose.h"

#ifdef STM32F103C8T6
    #include "stm32f10x.h"
#elif defined(ESP32_S3_SUPERMINI)
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
#endif

void main_board_choose_Init(void) {
    // Đánh thức tất cả các khối
    DRN_Timer_PWM_Init();
    DRN_Button_Init();
}

void main_board_choose_Delay_ms(unsigned int ms) {
#ifdef STM32F103C8T6
    for (volatile unsigned int i = 0; i < (ms * 8000); i++); 
#elif defined(ESP32_S3_SUPERMINI)
    vTaskDelay(ms / portTICK_PERIOD_MS);
#endif
}