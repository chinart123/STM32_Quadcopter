#include "i2c_mpu_debug.h"

void I2C1_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; 
    GPIOB->CRL &= ~(0xFF << 24); 
    GPIOB->CRL |= (0xFF << 24);
		GPIOB->ODR |= (1UL << 6) | (1UL << 7); //bật điện trở kéo lên nội bộ của STM32, một số lô GY-521 pháp sư Trung Hoa quên... hàn điện trở kéo lên (Pull-up resistor) cho 2 chân SCL và SDA
    RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST; 
    RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;
    I2C1->CR2 = 36; 
    I2C1->CCR = 180; 
    I2C1->TRISE = 37;
    I2C1->CR1 |= I2C_CR1_PE; 
}

// Hàm nội bộ dùng để Bật/Tắt nguồn
uint8_t MPU_WriteReg(uint8_t reg, uint8_t data) {
    uint32_t t = 10000;
    I2C1->CR1 |= I2C_CR1_START;
    while(!(I2C1->SR1 & I2C_SR1_SB)) { if(--t==0) return 0x05; }
    
    I2C1->DR = 0xD0; 
    t = 10000; while(!(I2C1->SR1 & I2C_SR1_ADDR)) { if(--t==0) return 0x06; }
    (void)I2C1->SR1; (void)I2C1->SR2;
    
    I2C1->DR = reg;  
    t = 10000; while(!(I2C1->SR1 & I2C_SR1_TXE)) { if(--t==0) return 0x07; }
    
    I2C1->DR = data; 
    t = 10000; while(!(I2C1->SR1 & I2C_SR1_BTF)) { if(--t==0) return 0x07; }
    
    I2C1->CR1 |= I2C_CR1_STOP;
    return 0x99;
}

void MPU_WakeUp(void) { MPU_WriteReg(0x6B, 0x00); }
void MPU_Sleep(void)  { MPU_WriteReg(0x6B, 0x40); }

void MPU_Read_WhoAmI(volatile uint8_t *state, volatile uint8_t *id) {
    uint32_t t = 10000;
    
    I2C1->CR1 |= I2C_CR1_START;
    while(!(I2C1->SR1 & I2C_SR1_SB)) { if(--t==0){ *state = 0x05; return;} }
    
    I2C1->DR = 0xD0; 
    t = 10000; while(!(I2C1->SR1 & I2C_SR1_ADDR)) { if(--t==0){ *state = 0x06; return;} }
    (void)I2C1->SR1; (void)I2C1->SR2;
    
    I2C1->DR = 0x75;  
    t = 10000; while(!(I2C1->SR1 & I2C_SR1_TXE)) { if(--t==0){ *state = 0x07; return;} }
    t = 10000; while(!(I2C1->SR1 & I2C_SR1_BTF)) { if(--t==0){ *state = 0x07; return;} }
    
    I2C1->CR1 |= I2C_CR1_START; 
    t = 10000; while(!(I2C1->SR1 & I2C_SR1_SB)) { if(--t==0){ *state = 0x05; return;} }
    
    I2C1->DR = 0xD1; 
    t = 10000; while(!(I2C1->SR1 & I2C_SR1_ADDR)) { if(--t==0){ *state = 0x06; return;} }
    
    I2C1->CR1 &= ~I2C_CR1_ACK; 
    I2C1->CR1 |= I2C_CR1_STOP; 
    (void)I2C1->SR1; (void)I2C1->SR2;
    
    t = 10000; while(!(I2C1->SR1 & I2C_SR1_RXNE)) { if(--t==0){ *state = 0x08; return;} }
    
    *id = I2C1->DR; 
    *state = 0x99; // Báo cáo PASS
}