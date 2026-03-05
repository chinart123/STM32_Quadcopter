#ifndef I2C_MPU_DEBUG_H
#define I2C_MPU_DEBUG_H

#include "stm32f10x.h"

// Khởi tạo phần cứng I2C
void I2C1_Init(void);

// Bật / Tắt nguồn mềm MPU6050
void MPU_WakeUp(void);
void MPU_Sleep(void);

// Đọc danh tính (WHO_AM_I) với mã lỗi:
// 0x05 (Start), 0x06 (Address), 0x07 (TXE/BTF), 0x08 (RXNE), 0x99 (PASS)
void MPU_Read_WhoAmI(volatile uint8_t *state, volatile uint8_t *id);

#endif