#include "hal_timer_pwm.h"

void HAL_TIM3_PWM_Init(void) {
    // 1. Cấp xung nhịp cho TIM3, Port B và Alternate Function
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;

    // 2. Cấu hình PB0 (CH3) và PB1 (CH4) là Alternate Function Push-Pull (50MHz)
    GPIOB->CRL &= ~((0xF << 0) | (0xF << 4)); // Xóa rác 8 bit đầu
    GPIOB->CRL |=  ((0xB << 0) | (0xB << 4)); // 0xB = 1011 (AF-PP)

    // 3. Cấu hình "Cây thước" (Clock = 72MHz)
    TIM3->PSC = 71;    // 1 bước đếm = 1 micro-giây (1us)
    TIM3->ARR = 999;   // Tổng chu kỳ = 1000 bước = 1 mili-giây (1kHz)

    // BẮT BUỘC 1: Ép phần cứng nạp cấu hình và xóa cờ rác (Trị bệnh kẹt Simulator) 
    // Nếu không có 2 dòng này, đôi khi cấu hình sẽ không được áp dụng ngay, dẫn đến việc PWM không hoạt động như mong đợi trên Simulator.
    // Cách hoạt động: Ghi 1 vào EGR (Event Generation Register) với bit UG (Update Generation) sẽ ép TIM3 nạp lại tất cả cấu hình từ PSC, ARR, CCMR, CCER... vào bộ đếm. Đồng thời, xóa cờ UIF (Update Interrupt Flag) trong SR để đảm bảo không có ngắt cập nhật nào bị kẹt lại.
    // Lưu ý: Trên phần cứng thật, việc này thường không cần thiết vì khi bạn bật TIM3, nó sẽ tự động nạp cấu hình. Tuy nhiên, trên Simulator, nếu bạn thấy PWM không hoạt động sau khi khởi tạo, hãy thử thêm 2 dòng này để đảm bảo cấu hình được áp dụng ngay.
    // Nếu bạn đang chạy trên phần cứng thật, bạn có thể bỏ qua 2 dòng này. Nhưng nếu bạn đang sử dụng Simulator và gặp vấn đề với PWM không hoạt động sau khi khởi tạo, hãy thêm 2 dòng này để đảm bảo cấu hình được áp dụng ngay.
    // Tóm lại: Trên phần cứng thật, bạn có thể bỏ qua 2 dòng này. Nhưng trên Simulator, nếu bạn thấy PWM không hoạt động sau khi khởi tạo, hãy thêm 2 dòng này để đảm bảo cấu hình được áp dụng ngay.
    TIM3->EGR |= TIM_EGR_UG;              
    TIM3->SR &= ~TIM_SR_UIF;
    // Giải thích thêm: Việc này giống như bạn "lắc" lại con đồng hồ sau khi chỉnh giờ, để đảm bảo nó chạy đúng theo thời gian mới. Trên phần cứng thật, con đồng hồ sẽ tự động nhận giờ mới khi bạn bật nó lên, nhưng trên Simulator, có thể cần phải "lắc" lại để nó nhận cấu hình mới.

    // 4. Cấu hình PWM Mode 1 cho Kênh 3 và 4 (Thanh ghi CCMR2)
    TIM3->CCMR2 &= ~((0xFF << 0) | (0xFF << 8)); // Reset rác
    TIM3->CCMR2 |= (6 << 4) | (6 << 12);         // OC3M = 110, OC4M = 110 (PWM Mode 1)
    TIM3->CCMR2 |= (1 << 3) | (1 << 11);         // Bật Preload (OC3PE, OC4PE)

    // 5. Cấu hình Output Enable (Thanh ghi CCER)
    // Mạch thực tế LED sáng ở mức 0 hay 1? Giả sử cấp mức 1 (Active High) thì LED sáng:
    TIM3->CCER &= ~((0xF << 8) | (0xF << 12));
    TIM3->CCER |= (1 << 8) | (1 << 12); // Bật công tắc CC3E và CC4E. (Cực tính mặc định CCxP=0)

    // Khởi tạo mức sáng mặc định = 0
    TIM3->CCR3 = 0;
    TIM3->CCR4 = 0;

    // 6. Cho phép đếm
    TIM3->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN;
}

void HAL_TIM3_PWM_SetDuty(uint8_t channel, uint16_t duty) {
    if (duty > 999) duty = 999; // Chặn trên, tránh tràn biến đếm
    
    // Ghi thẳng vào thanh ghi ranh giới CCR
    if (channel == 3) {
        TIM3->CCR3 = duty; 
    } else if (channel == 4) {
        TIM3->CCR4 = duty;
    }
}