#include "drn_time.h"
#include "drn_main_board_choose.h"

#ifdef STM32F103C8T6
    #include "stm32f10x.h"
    // Biến toàn cục đếm số mili-giây trôi qua (Chỉ dùng cho STM32)
    volatile uint32_t xx_sys_tick_ms = 0;
#elif defined(ESP32_S3_SUPERMINI)
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
#endif

void DRN_Time_Init(void) {
#ifdef STM32F103C8T6
    // Cấu hình SysTick ngắt mỗi 1ms (Clock hệ thống 72MHz)
    SysTick_Config(SystemCoreClock / 1000); 
#elif defined(ESP32_S3_SUPERMINI)
    // ESP32 dùng FreeRTOS nên hệ thống Tick đã tự động chạy ngầm, không cần Init
#endif
}

#ifdef STM32F103C8T6
// Trình phục vụ ngắt SysTick (Tự động nhảy vào đây mỗi 1ms)
void SysTick_Handler(void) {
    xx_sys_tick_ms++;
}
#endif

uint32_t DRN_Millis(void) {
#ifdef STM32F103C8T6
    return xx_sys_tick_ms;
#elif defined(ESP32_S3_SUPERMINI)
    // Lấy thời gian thực từ FreeRTOS quy đổi ra mili-giây
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
#endif
}