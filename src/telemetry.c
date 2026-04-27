#include "telemetry.h"
#include <stdio.h>

// --- BIẾN TOÀN CỤC NỘI BỘ MẢNG TELEMETRY ---
uint32_t last_csv_tick = 0;
uint32_t total_i2c_bytes = 0;
uint32_t i2c_bytes_in_last_sec = 0;
uint32_t i2c_speed_bps = 0;
uint32_t last_i2c_speed_tick = 0;

// Khởi tạo phần cứng UART1
void Telemetry_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPAEN;
    GPIOA->CRH &= ~(0xFF << 4); 
    GPIOA->CRH |= (0x0B << 4);  // PA9 TX
    GPIOA->CRH |= (0x04 << 8);  // PA10 RX
    USART1->BRR = 0x271;        // Baudrate 115200 @ 72MHz
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

// Chuyển hướng printf xuống cổng UART1
int fputc(int ch, FILE *f) {
    (void)f; // <--- FIX WARNING 1: Báo với Compiler là cố tình không dùng tham số này
    while (!(USART1->SR & USART_SR_TXE));
    USART1->DR = (ch & 0xFF);
    return ch;
}

void Telemetry_Print_Header(void) {
    printf("Time(ms),Roll,Pitch,Yaw,M1,M2,M3,M4,I2C_Speed(Bps),I2C_Volume(Bytes)\n"); 
}

void Telemetry_Reset_Counters(uint32_t current_tick) {
    total_i2c_bytes = 0;
    i2c_bytes_in_last_sec = 0;
    i2c_speed_bps = 0;
    last_i2c_speed_tick = current_tick;
}

void Telemetry_Update_I2C_Counter(uint16_t bytes) {
    total_i2c_bytes += bytes;
    i2c_bytes_in_last_sec += bytes;
}

void Telemetry_Calculate_Speed(uint32_t current_tick) {
    if (current_tick - last_i2c_speed_tick >= 1000) {
        i2c_speed_bps = i2c_bytes_in_last_sec; 
        i2c_bytes_in_last_sec = 0;             
        last_i2c_speed_tick = current_tick;
    }
}

void Telemetry_Send_CSV(uint32_t current_tick, MPU_Motion_t* imu, uint16_t m1, uint16_t m2, uint16_t m3, uint16_t m4) {
    if (current_tick - last_csv_tick >= 200) { // 5Hz (200ms)
        // <--- FIX WARNING 2 & 3: Đổi %lu thành %u ở cuối chuỗi định dạng
        printf("%u,%.2f,%.2f,%.2f,%d,%d,%d,%d,%u,%u\n", 
               current_tick,
               imu->Roll, imu->Pitch, imu->Yaw,
               m1, m2, m3, m4,
               i2c_speed_bps, total_i2c_bytes);
        last_csv_tick = current_tick;
    }
}