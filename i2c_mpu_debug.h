#ifndef I2C_MPU_DEBUG_H
#define I2C_MPU_DEBUG_H

#include "stm32f10x.h"

// =================================================================
// 1. TỪ ĐIỂN MÁY TRẠNG THÁI NÚT BẤM (Biến xx)
// Điều hướng luồng hoạt động của hệ thống qua PA0 và PA1
// =================================================================
// 0x01 : Kích hoạt (PA1) - Đánh thức cảm biến (Wake-up), chuẩn bị bus I2C.
// 0x02 : Thu thập (PA0 lần 1) - Đọc và cập nhật dữ liệu (Read & Update) liên tục.
// 0x03 : Xóa trắng (PA0 lần 2) - Dừng đọc, reset các biến hiển thị về 0 (Clear Data).
// 0x00 / 0x04 : Khóa lại (PA1) - Gửi lệnh Sleep, tắt cảm biến để tiết kiệm năng lượng.
// =================================================================

// =================================================================
// 2. TỪ ĐIỂN TRẠNG THÁI I2C (Biến xx_mpu_state)
// Giám sát sức khỏe đường truyền I2C theo thời gian thực
// =================================================================
// 0x00 : Trạng thái khởi tạo / Đã reset.
// 0x05 : [LỖI VẬT LÝ] Treo Bus I2C (Cần gọi hàm I2C_Bus_Recovery).
// 0x06 : [LỖI GIAO TIẾP] Không tìm thấy địa chỉ MPU (Gõ cửa không ai thưa).
// 0x07 : [LỖI TRUYỀN TẢI] Kẹt ở khâu gửi dữ liệu (TXE/BTF Timeout).
// 0x08 : [LỖI NHẬN DATA] MPU không chịu nhả dữ liệu (Silicon Bug 1-Byte).
// 0x99 : [THÀNH CÔNG] Đường truyền thông suốt, dữ liệu an toàn.
// =================================================================

// Các biến toàn cục để ghim lên cửa sổ Watch
extern volatile uint8_t xx;           // Biến điều hướng vòng lặp
extern volatile uint8_t xx_mpu_state; // Biến trạng thái I2C
extern volatile uint8_t xx_mpu_id;    // ID của cảm biến (Kỳ vọng: 0x70 với MPU-6500)

// Các hàm giao tiếp cơ bản
void I2C1_Init(void);
void MPU_WakeUp(void);
void MPU_Sleep(void);
void MPU_Read_WhoAmI(volatile uint8_t *state, volatile uint8_t *id);

// Các hàm quét dữ liệu
void MPU_Read_Multi(uint8_t reg, uint8_t *data, uint8_t size);
uint8_t MPU_WriteReg(uint8_t reg, uint8_t data);
#endif