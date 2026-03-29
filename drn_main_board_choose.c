#include "drn_main_board_choose.h"

#ifdef ESP32_S3_SUPERMINI
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
#endif

void drn_main_board_choose_Init(void) {
    DRN_Time_Init();
    DRN_Motor_PWM_Init();
    DRN_Button_Init();
}

void drn_main_board_choose_Delay_ms(unsigned int ms) {
#ifdef STM32F103C8T6
    uint32_t start_time = DRN_Millis();
    while ((DRN_Millis() - start_time) < ms) {
        // Đợi bằng SysTick thay vì vòng lặp rỗng
    }
#elif defined(ESP32_S3_SUPERMINI)
    vTaskDelay(ms / portTICK_PERIOD_MS);
#endif
}