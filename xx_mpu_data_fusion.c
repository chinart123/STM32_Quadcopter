#include "xx_mpu_data_fusion.h"
#include "i2c_mpu_debug.h" // Gọi tài xế I2C vào làm việc
#include <math.h> // Thư viện toán học cực mạnh để tính Lượng giác
// Khởi tạo biến toàn cục đã khai báo ở file .h
MPU_Motion_t Drone_IMU;

// (Nhắc nhẹ: Sếp nhớ đảm bảo hàm MPU_WriteReg đã được khai báo trong i2c_mpu_debug.h nhé, 
// nếu chưa có thì sếp copy dòng này ném vào file .h đó: uint8_t MPU_WriteReg(uint8_t reg, uint8_t data); )
extern uint8_t MPU_WriteReg(uint8_t reg, uint8_t data);

// Cần gọi tài xế I2C từ file kia sang
extern void MPU_Read_Multi(uint8_t reg, uint8_t *data, uint8_t size);

// =================================================================
// HÀM CẤU HÌNH MPU-6500 CHUYÊN DỤNG CHO QUADCOPTER
// =================================================================
void MPU_Fusion_Init(void) {
    // 1. Đánh thức cảm biến (Reset thanh ghi Nguồn)
    // Thanh ghi 0x6B (PWR_MGMT_1) -> Ghi 0x00: Dùng xung nhịp nội bộ 8MHz, tắt Sleep.
    MPU_WriteReg(0x6B, 0x00);
    
    // Đợi 1 chút cho cảm biến ổn định dao động ký (Tầm 10ms)
    for(volatile int i=0; i<72000; i++); 

    // 2. Bật Bộ lọc thông thấp kỹ thuật số (DLPF - Digital Low Pass Filter)
    // Thanh ghi 0x1A (CONFIG) -> Ghi 0x03: Lọc nhiễu ở tần số ~41Hz. 
    // Nếu không có dòng này, lúc động cơ quay mạch sẽ rung bần bật, nhiễu không bay nổi!
    MPU_WriteReg(0x1A, 0x03);

    // 3. Cấu hình tầm đo Con quay hồi chuyển (Gyroscope Full Scale Range)
    // Thanh ghi 0x1B (GYRO_CONFIG) -> Ghi 0x08: Chọn dải ±500 độ/giây (dps).
    // Tầm này đủ để Quadcopter nhào lộn mượt mà. Hệ số chia tương ứng là 65.5 LSB/dps.
    MPU_WriteReg(0x1B, 0x08);

    // 4. Cấu hình tầm đo Gia tốc kế (Accelerometer Full Scale Range)
    // Thanh ghi 0x1C (ACCEL_CONFIG) -> Ghi 0x10: Chọn dải ±8g.
    // Chịu được gia tốc gấp 8 lần trọng trường Trái Đất (Phù hợp để bốc đầu). Hệ số chia là 4096 LSB/g.
    MPU_WriteReg(0x1C, 0x10);
    
    // Khởi tạo Struct về 0 cho sạch sẽ
    Drone_IMU.Roll = 0.0f;
    Drone_IMU.Pitch = 0.0f;
    Drone_IMU.Yaw = 0.0f;
}

// =================================================================
// 1. HÀM HÚT 14 BYTES TỪ MPU-6500 (Thanh ghi 0x3B đến 0x48)
// =================================================================
void MPU_Fusion_Read_Burst(void) {
    uint8_t buffer[14];
    
    // Gọi tài xế I2C chạy ra địa chỉ 0x3B (Gia tốc X), hút 14 byte mang về
    MPU_Read_Multi(0x3B, buffer, 14);
    
    // Ép kiểu (Ghép 2 byte 8-bit thành 1 số nguyên có dấu 16-bit)
    Drone_IMU.Accel_X_RAW = (int16_t)((buffer[0] << 8) | buffer[1]);
    Drone_IMU.Accel_Y_RAW = (int16_t)((buffer[2] << 8) | buffer[3]);
    Drone_IMU.Accel_Z_RAW = (int16_t)((buffer[4] << 8) | buffer[5]);
    
    Drone_IMU.Temp_RAW    = (int16_t)((buffer[6] << 8) | buffer[7]);
    
    Drone_IMU.Gyro_X_RAW  = (int16_t)((buffer[8] << 8) | buffer[9]);
    Drone_IMU.Gyro_Y_RAW  = (int16_t)((buffer[10] << 8) | buffer[11]);
    Drone_IMU.Gyro_Z_RAW  = (int16_t)((buffer[12] << 8) | buffer[13]);
}

// =================================================================
// 2. CỐI XAY TOÁN HỌC (CHUYỂN ĐỔI SỐ LIỆU VÀ TÍNH GÓC)
// =================================================================
void MPU_Fusion_Compute(void) {
    // Bước 1: Scale Dữ liệu (Chia cho hằng số phần cứng)
    // Accel chia 4096 (Cấu hình ±8g). Gyro chia 65.5 (Cấu hình ±500 dps)
    Drone_IMU.Ax = (float)Drone_IMU.Accel_X_RAW / 4096.0f;
    Drone_IMU.Ay = (float)Drone_IMU.Accel_Y_RAW / 4096.0f;
    Drone_IMU.Az = (float)Drone_IMU.Accel_Z_RAW / 4096.0f;

    Drone_IMU.Gx = (float)Drone_IMU.Gyro_X_RAW / 65.5f;
    Drone_IMU.Gy = (float)Drone_IMU.Gyro_Y_RAW / 65.5f;
    Drone_IMU.Gz = (float)Drone_IMU.Gyro_Z_RAW / 65.5f;

    // Bước 2: Dùng Toán học giải tích (Trigonometry) tính góc Roll & Pitch từ Gia tốc
    // Hệ số 57.2957795f dùng để đổi từ đơn vị Radian sang Độ (180/PI)
    Drone_IMU.Roll  = atan2(Drone_IMU.Ay, Drone_IMU.Az) * 57.2957795f;
    
    // Dùng công thức Pythagoras cho trục Pitch để triệt tiêu nhiễu khi xoay ngược
    Drone_IMU.Pitch = atan2(-Drone_IMU.Ax, sqrt(Drone_IMU.Ay * Drone_IMU.Ay + Drone_IMU.Az * Drone_IMU.Az)) * 57.2957795f;
    
    // (Góc Yaw rất phức tạp vì bị Drift (trôi), tạm thời anh em mình chưa tính ở bước này)
}
/*
2. Giải mã "Ma Thuật Trắng" (Register Map)
Sếp nhìn vào mấy con số 0x1A, 0x1B, 0x1C có thấy lú không? 
Đừng lo, tất cả đều được anh lấy từ "Kinh Thánh" (Datasheet) của hãng InvenSense ra cả đấy:
DLPF (0x1A = 0x03): Cánh quạt Quadcopter thường rung ở tần số rất cao (hàng trăm Hz). 
Bật bộ lọc mức số 3 (0x03) sẽ chém bỏ toàn bộ những cái rung động láo nháo đó, chỉ giữ lại chuyển động nghiêng thực sự của thân máy bay (dưới 41Hz).
Gyro (0x1B = 0x08): Mặc định cảm biến chỉ đo được ±250 độ/giây. 
Quadcopter lật rất nhanh, nếu để mặc định sẽ bị "chạm trần" (kẹt số). 
Nâng lên ±500 dps là con số vàng cho các mạch Flight Controller hiện nay.
Accel (0x1C = 0x10): Mặc định là ±2g (Quá yếu, rơi tự do hoặc thắng gấp là kẹt số). 
Nâng lên ±8g là máy bay rớt cái ạch vẫn đo được thông số.

3. Kịch bản Test ở Main
Để kiểm chứng hàm này không làm treo hệ thống, sếp mở main.c ra, tìm đến cái case 0x01: (Trạng thái Wake-up bằng nút PA1) và nâng cấp nó lên:
C
// ... trong vòng lặp switch(xx) ...
case 0x01: // BẤM PA1 - KHỞI ĐỘNG HỆ THỐNG
    MPU_Fusion_Init(); // Gọi hàm cấu hình cực xịn vừa viết
    MPU_Read_WhoAmI(&xx_mpu_state, &xx_mpu_id); // Đọc ID xem còn sống không
    
    // Nếu sếp thích hiển thị trực quan, có thể chớp tắt LED xanh 1 cái ở đây để báo Init xong
    break;
*/