#ifndef XX_MPU_DATA_FUSION_H
#define XX_MPU_DATA_FUSION_H

#include "stm32f10x.h"
/*
Ý đồ của bản thiết kế này:
Sếp có thấy anh chia cái struct ra làm 3 nhóm rất rõ ràng không?
Nhóm 1 (RAW): Hứng nước mưa (14 byte I2C).
Nhóm 2 (Scaled): Lọc cặn (Đổi từ tín hiệu điện ra số đo vật lý).
Nhóm 3 (Fused): Nước tinh khiết (Góc bay chuẩn xác không nhiễu để nạp vào hệ thống Motor).
Sau này lúc test, sếp chỉ việc gõ đúng chữ Drone_IMU vào cửa sổ Watch 1, mở dấu + ra là sếp kiểm soát được toàn bộ từ lúc data mới chui vào cho đến khi nó biến thành góc bay.
*/
// =================================================================
// CẤU TRÚC DỮ LIỆU ĐIỀU KHIỂN BAY (FLIGHT DYNAMICS STRUCT)
// Gói gọn toàn bộ sinh mạng của Quadcopter vào một chỗ để dễ Debug
// =================================================================
typedef struct {
    // 1. Nhóm Dữ liệu Thô (Raw Data) - Hút trực tiếp 14 bytes từ I2C
    // Kiểu int16_t (có dấu) để biểu diễn được cả chiều âm/dương của trục
    int16_t Accel_X_RAW;
    int16_t Accel_Y_RAW;
    int16_t Accel_Z_RAW;
    
    int16_t Temp_RAW;      // Nhiệt độ lõi chip (Đọc cho vui, ít dùng để bay)
    
    int16_t Gyro_X_RAW;
    int16_t Gyro_Y_RAW;
    int16_t Gyro_Z_RAW;

    // 2. Nhóm Dữ liệu Vật lý (Scaled Data) - Đã chia cho hằng số độ nhạy
    float Ax, Ay, Az;      // Gia tốc thực tế (đơn vị: g)
    float Gx, Gy, Gz;      // Tốc độ góc thực tế (đơn vị: độ/giây - dps)

    // 3. Nhóm Góc Bay Thực Tế (Fused Data) - Đã qua bộ lọc Toán học
    float Roll;            // Góc nghiêng trái/phải (Oanh tạc)
    float Pitch;           // Góc ngóc đầu/chúi mũi (Bốc đầu)
    float Yaw;             // Góc xoay quanh trục Z (Đảo đuôi)
    
} MPU_Motion_t;

// =================================================================
// KHAI BÁO BIẾN TOÀN CỤC & HÀM XỬ LÝ
// =================================================================
// Biến duy nhất sếp cần lôi ra cửa sổ Watch 1 để ngắm
extern MPU_Motion_t Drone_IMU; 

// Các hàm sẽ viết trong file .c
void MPU_Fusion_Init(void);       // Ghi cấu hình (DLPF, Range) vào MPU6500
void MPU_Fusion_Read_Burst(void); // Hút 14 bytes thô bằng 1 lệnh I2C
void MPU_Fusion_Compute(void);    // Chạy cối xay toán học (Complementary Filter)

#endif