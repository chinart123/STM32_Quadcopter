#include "i2c_mpu_debug.h"
 
// ==========================================
// STAGE 4: Code hoàn thiện 100%. 
// Đã test thực tế với MPU-6500 (ID: 0x70)
// ==========================================

// Hàm chọc ngoáy thủ công để gỡ kẹt Bus I2C
void I2C_Bus_Recovery(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    GPIOB->CRL &= ~(0xFF << 24);
    GPIOB->CRL |= (0x77 << 24); 
    GPIOB->ODR |= (1 << 7); 
    for(int i = 0; i < 9; i++) {
        GPIOB->ODR |= (1 << 6); 
        for(volatile int j = 0; j < 2000; j++); 
        GPIOB->ODR &= ~(1 << 6); 
        for(volatile int j = 0; j < 2000; j++);
    }
    GPIOB->ODR &= ~(1 << 7); 
    for(volatile int j = 0; j < 2000; j++);
    GPIOB->ODR |= (1 << 6);  
    for(volatile int j = 0; j < 2000; j++);
    GPIOB->ODR |= (1 << 7);  
    for(volatile int j = 0; j < 2000; j++);
}

void I2C1_Init(void) {
    I2C_Bus_Recovery(); // Quét sạch rác trên dây SDA
    
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; 
    GPIOB->CRL &= ~(0xFF << 24); 
    GPIOB->CRL |= (0xFF << 24);  
    
    RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST; 
    RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;
    
    // --- KHẮC PHỤC LỖI TRÀN BIT DO ÉP XUNG 128MHz ---
    uint32_t apb1_freq = SystemCoreClock / 2; 
    uint32_t freq_mhz = apb1_freq / 1000000;
    
    if (freq_mhz > 50) freq_mhz = 50; 
    if (freq_mhz == 0) freq_mhz = 8;  
    
    I2C1->CR2 = freq_mhz; 
    I2C1->CCR = (apb1_freq / 100000) / 2; 
    I2C1->TRISE = freq_mhz + 1;
    
    I2C1->CR1 |= I2C_CR1_PE; 
    I2C1->CR1 |= I2C_CR1_ACK; 
}

uint8_t MPU_WriteReg(uint8_t reg, uint8_t data) {
    uint32_t t = 100000; 
    I2C1->CR1 |= I2C_CR1_START;
    while(!(I2C1->SR1 & I2C_SR1_SB)) { if(--t==0) return 0x05; }
    
    I2C1->DR = 0xD0; 
    t = 100000; while(!(I2C1->SR1 & I2C_SR1_ADDR)) { if(--t==0) return 0x06; }
    (void)I2C1->SR1; (void)I2C1->SR2;
    
    I2C1->DR = reg;  
    t = 100000; while(!(I2C1->SR1 & I2C_SR1_TXE)) { if(--t==0) return 0x07; }
    
    I2C1->DR = data; 
    t = 100000; while(!(I2C1->SR1 & I2C_SR1_BTF)) { if(--t==0) return 0x07; }
    
    I2C1->CR1 |= I2C_CR1_STOP;
    return 0x99;
}

void MPU_WakeUp(void) { MPU_WriteReg(0x6B, 0x00); }
void MPU_Sleep(void)  { MPU_WriteReg(0x6B, 0x40); }

void MPU_Read_WhoAmI(volatile uint8_t *state, volatile uint8_t *id) {
    // RESET LẠI I2C TRƯỚC MỖI LẦN ĐỌC
    I2C1_Init(); 
    
    uint32_t t = 100000;
    
    I2C1->CR1 |= I2C_CR1_START;
    while(!(I2C1->SR1 & I2C_SR1_SB)) { if(--t==0){ *state = 0x05; return;} }
    
    I2C1->DR = 0xD0; 
    t = 100000; while(!(I2C1->SR1 & I2C_SR1_ADDR)) { if(--t==0){ *state = 0x06; return;} }
    (void)I2C1->SR1; (void)I2C1->SR2;
    
    I2C1->DR = 0x75;  
    t = 100000; while(!(I2C1->SR1 & I2C_SR1_TXE)) { if(--t==0){ *state = 0x07; return;} }
    t = 100000; while(!(I2C1->SR1 & I2C_SR1_BTF)) { if(--t==0){ *state = 0x07; return;} }
    
    I2C1->CR1 |= I2C_CR1_START; 
    t = 100000; while(!(I2C1->SR1 & I2C_SR1_SB)) { if(--t==0){ *state = 0x05; return;} }
    
    I2C1->DR = 0xD1; 
    t = 100000; while(!(I2C1->SR1 & I2C_SR1_ADDR)) { if(--t==0){ *state = 0x06; return;} }
    
    // =================================================================
    // ĐOẠN XÓA CỜ CHUẨN ĐẾN TỪNG NANO-GIÂY CỦA STM32F1
    // =================================================================
    I2C1->CR1 &= ~I2C_CR1_ACK;        // Tắt ACK TRƯỚC
    (void)I2C1->SR1; (void)I2C1->SR2; // Ép đọc SR1, SR2 để xóa cờ ADDR
    I2C1->CR1 |= I2C_CR1_STOP;        // Cài cờ STOP NGAY SAU KHI XÓA ADDR
    // =================================================================
    
    t = 100000; 
    while(!(I2C1->SR1 & I2C_SR1_RXNE)) { if(--t==0){ *state = 0x08; return;} }
    
    *id = I2C1->DR; 
    *state = 0x99; 
}

// [FIX LỖI]: Đổi void thành uint8_t để biết đọc Thành công hay Thất bại
// =================================================================
// STAGE 7: HÀM ĐỌC NHIỀU BYTE - BẢN "CỖ XE TĂNG" (KHÔNG MACRO)
// =================================================================
uint8_t MPU_Read_Multi(uint8_t reg, uint8_t *data, uint8_t size) {
    uint32_t timeout;

    // [FIX LỖI]: Tăng Timeout lên 500.000 (~20ms) để không chém nhầm do SysTick ngắt
    timeout = 500000;
    while(I2C1->SR2 & I2C_SR2_BUSY) { if(--timeout == 0) goto i2c_error; }

    I2C1->CR1 |= I2C_CR1_START;
    timeout = 500000;
    while(!(I2C1->SR1 & I2C_SR1_SB)) { if(--timeout == 0) goto i2c_error; }
    
    I2C1->DR = 0x68 << 1; 
    timeout = 500000;
    while(!(I2C1->SR1 & I2C_SR1_ADDR)) { if(--timeout == 0) goto i2c_error; }
    (void)I2C1->SR2; 
    
    I2C1->DR = reg;
    timeout = 500000;
    while(!(I2C1->SR1 & I2C_SR1_TXE)) { if(--timeout == 0) goto i2c_error; }
    
    I2C1->CR1 |= I2C_CR1_START;
    timeout = 500000;
    while(!(I2C1->SR1 & I2C_SR1_SB)) { if(--timeout == 0) goto i2c_error; }
    
    I2C1->DR = (0x68 << 1) | 1; 
    timeout = 500000;
    while(!(I2C1->SR1 & I2C_SR1_ADDR)) { if(--timeout == 0) goto i2c_error; }
    (void)I2C1->SR2; 
    
    I2C1->CR1 |= I2C_CR1_ACK;
    for(uint8_t i = 0; i < size; i++) {
        if(i == size - 1) { I2C1->CR1 &= ~I2C_CR1_ACK; I2C1->CR1 |= I2C_CR1_STOP; }
        timeout = 500000;
        while(!(I2C1->SR1 & I2C_SR1_RXNE)) { if(--timeout == 0) goto i2c_error; }
        data[i] = I2C1->DR;
    }
    
    xx_mpu_state = 0x99; 
    return 1; // [FIX LỖI]: Trả về 1 báo cáo HÚT THÀNH CÔNG

i2c_error:
    xx_mpu_state = 0x08; 
    I2C_Bus_Recovery();  
    I2C1_Init();         
    return 0; // [FIX LỖI]: Trả về 0 báo cáo ĐỨT DÂY, ngăn chặn tính toán rác
}

// =================================================================
// THÊM MỚI Ở STAGE 7: KHỞI TẠO BỘ ĐẾM MICRO-GIÂY (TIM4)
// =================================================================
void HAL_TIM4_Micros_Init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
    TIM4->PSC = 71; 
    TIM4->ARR = 0xFFFF;
    TIM4->EGR |= TIM_EGR_UG;
    TIM4->CR1 |= TIM_CR1_CEN;
}

uint16_t micros(void) {
    return TIM4->CNT;
}