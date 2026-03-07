# TÀI LIỆU KIẾN TRÚC HỆ THỐNG: MPU-6500 DATA FUSION (V1.0)

## 1. Cấu trúc File (System Architecture)
Hệ thống được thiết kế theo mô hình Phân tầng (Layered Architecture):
* **`main.c`**: Trái tim điều phối. Chứa Cỗ máy trạng thái (FSM) phân cấp Cha-Con để quản lý luồng thực thi dựa trên tín hiệu nút bấm.
* **`button.c / .h`**: Tầng Giao diện (UI). Chống dội nút bấm (Debounce) và phát hiện sự kiện Click.
* **`i2c_mpu_debug.c / .h`**: Tầng Giao tiếp Phần cứng (HAL). Xử lý giao thức I2C, gửi lệnh Đọc/Ghi, quản lý lỗi đường truyền và cấu hình Nguồn (Wake/Sleep).
* **`xx_mpu_data_fusion.c / .h`**: Tầng Ứng dụng & Toán học (DSP). Nạp thông số bay (DLPF, Range), hứng dữ liệu thô (Burst Read 14 bytes) và chạy thuật toán tính góc bay (Roll, Pitch, Yaw).

---

## 2. Từ điển Biến số (Variable Dictionary)
### Biến Điều hướng & Trạng thái (System States)
* `xx` (uint8_t): Biến điều khiển Cỗ máy trạng thái (FSM) chính.
* `xx_mpu_state` (uint8_t): Mã trạng thái đường truyền I2C (0x99 = OK, 0x05-0x08 = Lỗi vật lý/giao tiếp).
* `xx_mpu_id` (uint8_t): Lưu mã định danh của cảm biến (Kỳ vọng: 0x70 cho MPU-6500).

### Cấu trúc Dữ liệu Bay (`Drone_IMU` - Kiểu `MPU_Motion_t`)
* **Dữ liệu thô (Raw - int16_t):** `Accel_X_RAW`, `Gyro_Z_RAW`, `Temp_RAW`... (Chưa qua xử lý).
* **Dữ liệu vật lý (Scaled - float):** `Ax, Ay, Az` (đơn vị: g) và `Gx, Gy, Gz` (đơn vị: độ/giây).
* **Góc bay thực tế (Fused - float):** `Roll` (Nghiêng trái/phải), `Pitch` (Ngóc/chúi), `Yaw` (Xoay Z).

---

## 3. Cỗ Máy Trạng Thái Phân Cấp (Hierarchical FSM)
Hệ thống sử dụng FSM phân cấp quyền lực để đảm bảo an toàn tuyệt đối khi vận hành:

### LỚP CHA: NÚT PA1 (Quyền Sinh Sát)
* **`xx = 0x01` (INIT / STANDBY):** Bật nguồn, nạp cấu hình (±8g, ±500dps, DLPF), sẵn sàng chờ lệnh.
* **`xx = 0x04` (SLEEP):** Tắt nguồn cảm biến, đóng băng toàn bộ dữ liệu hiện trường, Reset các biến góc bay về 0 để an toàn.

### LỚP CON: NÚT PA0 (Chức năng Đo lường)
*(Chỉ hoạt động khi Lớp Cha đang ở trạng thái BẬT - `0x01`)*
* **`xx = 0x02` (CONTINUOUS STREAM - XẢ LŨ):** Vắt kiệt sức mạnh CPU, liên tục hút 14 byte dữ liệu và tính toán góc bay theo thời gian thực.
* **`xx = 0x03` (SNAPSHOT - CHỤP MÀN HÌNH):** Dừng vòng lặp đọc dữ liệu. Đóng băng toàn bộ biến trên RAM để kỹ sư tra cứu dấu (+/-) hoặc quan sát hiện trường tĩnh.