#ifndef XX_MPU_COMPLEMENTARY_FILTER_H
#define XX_MPU_COMPLEMENTARY_FILTER_H

#include "stm32f10x.h"
#include <stdio.h> // Dùng cho printf

// Cấu trúc Máy lọc đã khôi phục các biến Debug cho Sếp
typedef struct {
    // 1. Dữ liệu thô
    int16_t Accel_X_RAW, Accel_Y_RAW, Accel_Z_RAW;
    int16_t Gyro_X_RAW,  Gyro_Y_RAW,  Gyro_Z_RAW;

    // --- BIẾN DEBUG ĐƯỢC GIỮ LẠI ---
    int16_t Accel_X_Offset, Accel_Y_Offset, Accel_Z_Offset;
    int16_t Gyro_X_Offset,  Gyro_Y_Offset,  Gyro_Z_Offset;

    // 2. Dữ liệu vật lý (g, dps)
    float Ax, Ay, Az;
    float Gx, Gy, Gz;

    // 3. Thời gian và Góc bay
    float dt;
    uint16_t last_time;
    float Roll, Pitch, Yaw;
    
    // --- THỐNG KÊ LƯU LƯỢNG DATA ---
    uint32_t Bytes_Per_Second; 
    
} DMPU_Motion_t_complementary_filter;

extern DMPU_Motion_t_complementary_filter Filter_IMU;

void    Filter_Machine_UART_Init(void); // Bật cổng Serial (Baudrate 115200)
void    Filter_Machine_Init(void);      
uint8_t Filter_Machine_Calibrate(void); 
uint8_t Filter_Machine_Run(void);       

#endif