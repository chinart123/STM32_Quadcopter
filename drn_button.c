#include "drn_button.h"
#include "main_board_choose.h"

#ifdef STM32F103C8T6
    #include "stm32f10x.h"
#elif defined(ESP32_S3_SUPERMINI)
    // #include "driver/gpio.h"
#endif

void DRN_Button_Init(void) {
#ifdef STM32F103C8T6
    // 1. Cấp Clock cho Port A
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    // 2. Cấu hình PA0 và PA1 làm Input Pull-up / Pull-down
    // Xóa cấu hình cũ của chân 0 (bit 0-3) và chân 1 (bit 4-7)
    GPIOA->CRL &= ~((0xF << 0) | (0xF << 4)); 
    
    // Nạp giá trị 0x8 (1000) vào để làm Input mode
    GPIOA->CRL |=  ((0x8 << 0) | (0x8 << 4)); 
    
    // 3. Đẩy thanh ghi ODR lên 1 để chốt chế độ Pull-Up (Mặc định ở mức Cao 3.3V)
    // Bit 0 cho PA0, Bit 1 cho PA1
    GPIOA->ODR |= (1 << 0) | (1 << 1);    
#elif defined(ESP32_S3_SUPERMINI)
    // Code ESP32 GPIO Init...
#endif
}

uint8_t DRN_Button_Read_PA0(void) {
#ifdef STM32F103C8T6
    // Đọc bit thứ 0 của thanh ghi IDR. 
    // Nếu bằng 0 tức là người dùng đang nhấn nút (kéo mạch xuống GND).
    if ((GPIOA->IDR & (1 << 0)) == 0) {
        return 1; // Phát hiện có nhấn
    } else {
        return 0; // Đang nhả
    }
#elif defined(ESP32_S3_SUPERMINI)
    return 0;
#endif
}

uint8_t DRN_Button_Read_PA1(void) {
#ifdef STM32F103C8T6
    // Đọc bit thứ 1 của thanh ghi IDR.
    if ((GPIOA->IDR & (1 << 1)) == 0) {
        return 1; // Phát hiện có nhấn
    } else {
        return 0; // Đang nhả
    }
#elif defined(ESP32_S3_SUPERMINI)
    return 0;
#endif
}