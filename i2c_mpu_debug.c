#include "i2c_mpu_debug.h"

// Hàm chọc ngoáy thủ công để gỡ kẹt Bus I2C (Bus Recovery)
void I2C_Bus_Recovery(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    
    // Cấu hình PB6 (SCL) và PB7 (SDA) là Output Open-Drain (Không dùng Alternate Function)
    GPIOB->CRL &= ~(0xFF << 24);
    GPIOB->CRL |= (0x77 << 24); 
    
    GPIOB->ODR |= (1 << 7); // Kéo SDA lên High
    
    // Tạo 9 xung Clock thủ công để ép Slave nhả Bus
    for(int i = 0; i < 9; i++) {
        GPIOB->ODR |= (1 << 6); // SCL High
        for(volatile int j = 0; j < 2000; j++); 
        GPIOB->ODR &= ~(1 << 6); // SCL Low
        for(volatile int j = 0; j < 2000; j++);
    }
    
    // Tạo giả lập tín hiệu STOP (SCL lên High, sau đó SDA lên High)
    GPIOB->ODR &= ~(1 << 7); // SDA Low
    for(volatile int j = 0; j < 2000; j++);
    GPIOB->ODR |= (1 << 6);  // SCL High
    for(volatile int j = 0; j < 2000; j++);
    GPIOB->ODR |= (1 << 7);  // SDA High
    for(volatile int j = 0; j < 2000; j++);
}

void I2C1_Init(void) {
    // 1. Tự động thông tắc Bus trước khi khởi tạo
    I2C_Bus_Recovery();
    
    // 2. Cấp xung nhịp
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; 
    
    // 3. Trả PB6, PB7 về đúng chế độ Alternate Function Open-Drain
    GPIOB->CRL &= ~(0xFF << 24); 
    GPIOB->CRL |= (0xFF << 24);  
    
    // 4. Reset bộ I2C1
    RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST; 
    RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;
    
    // 5. Tính toán Tự động theo Xung nhịp hệ thống (Chống lỗi ép xung)
    uint32_t pclk1 = SystemCoreClock / 2; // Bus APB1 thường bằng 1/2 SysClock
    uint32_t freq_mhz = pclk1 / 1000000;
    
    I2C1->CR2 = freq_mhz; 
    I2C1->CCR = pclk1 / (100000 * 2); // Chuẩn 100kHz
    I2C1->TRISE = freq_mhz + 1;
    
    I2C1->CR1 |= I2C_CR1_PE; 
}

uint8_t MPU_WriteReg(uint8_t reg, uint8_t data) {
    uint32_t t = 100000; // Tăng kịch kim Timeout
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
    
    I2C1->CR1 &= ~I2C_CR1_ACK; 
    I2C1->CR1 |= I2C_CR1_STOP; 
    (void)I2C1->SR1; (void)I2C1->SR2;
    
    t = 100000; while(!(I2C1->SR1 & I2C_SR1_RXNE)) { if(--t==0){ *state = 0x08; return;} }
    
    *id = I2C1->DR; 
    *state = 0x99; 
}