#ifndef __TELEMETRY_H
#define __TELEMETRY_H

#include "stm32f10x.h"
#include "xx_mpu_data_fusion.h" // Để lấy struct MPU_Motion_t

// Khởi tạo UART1 và cấu hình chân PA9 (TX)
void Telemetry_Init(void);

// In tiêu đề các cột cho file CSV
void Telemetry_Print_Header(void);

// Reset các biến đếm I2C khi bắt đầu bay
void Telemetry_Reset_Counters(uint32_t current_tick);

// Cộng dồn số byte I2C vừa kéo được
void Telemetry_Update_I2C_Counter(uint16_t bytes);

// Tính toán tốc độ Bps mỗi 1 giây
void Telemetry_Calculate_Speed(uint32_t current_tick);

// Xuất chuỗi CSV định kỳ (5Hz)
void Telemetry_Send_CSV(uint32_t current_tick, MPU_Motion_t* imu, uint16_t m1, uint16_t m2, uint16_t m3, uint16_t m4);

#endif