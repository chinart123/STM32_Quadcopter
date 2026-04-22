# NHẬT KÝ DỰ ÁN QUADCUPTER (PHẦN 1): NỀN TẢNG STM32 & BÀI HỌC THỰC NGHIỆM
**Đối tượng:** STM32F103C8T6 (Bluepill)
**Kiến trúc:** Bare-metal (Vòng lặp tuần tự - Polling)

## 1. PHÂN TÍCH TÌNH HÌNH VÀ CÁC BIẾN CỐ PHẦN CỨNG
Giai đoạn STM32 là quá trình "thử sai" quan trọng để xác định các giới hạn vật lý của linh kiện trong hệ thống động lực.

### 1.1. Thất bại của Mạch Công suất (MOSFET & Diode)
* **Sự cố Thủng lớp cách điện Gate (Gate Oxide Breakdown):** Do mắc Diode nối tiếp sai nguyên lý. 
* **Hậu quả:** Xung suất điện động cảm ứng (Flyback voltage) từ motor 8520 vọt lên hàng trăm vôn, dội ngược qua tụ ký sinh $C_{rss}$ vào chân Gate. 
* **Dấu hiệu:** Chân Gate bị chập vĩnh viễn với Drain/Source, đo nội trở nhảy loạn xạ và tụt về $0\Omega$.
* **Giải pháp:** Mắc Diode SS54 song song ngược chiều với motor để ghim áp dội ở mức an toàn ~5.1V.

### 1.2. Khủng hoảng Giao tiếp I2C (Blocking Architecture)
* **Lỗi Silicon & Bus:** Gặp các lỗi `0x05` (SDA Stuck Low) và `0x08` (Silicon Bug 1-Byte) của STM32.
* **Cơ chế gỡ kẹt:** Phải viết hàm `I2C_Bus_Recovery` nhấp nhả thủ công chân SCL 9 lần (bit-banging) để ép MPU nhả chân SDA.
* **Hạn chế:** Code dùng vòng lặp `while` đếm lùi $t=100000$ để thoát lỗi. Việc này "ngốn" sạch quỹ thời gian (Budget Time) của vòng lặp 2ms, làm hệ thống mất tính thời gian thực.

## 2. BẢNG THÔNG SỐ KỸ THUẬT (STM32)

| Thông số | Giá trị cấu hình | Nhận định kỹ thuật |
| :--- | :--- | :--- |
| **Xung nhịp CPU** | 72 MHz | Giới hạn tốc độ tính toán số thực (Float). |
| **Tần số PWM** | 500Hz đến 4kHz | Gây tiếng rít cơ khí, Mosfet mát nhưng chổi than motor nhanh mòn. |
| **Độ phân giải thời gian** | 1 ms (SysTick) | Quá thô để tính toán sai phân ($dt$) cho PID mượt mà. |
| **Chế độ đọc MPU** | Polling (Hỏi vòng) | CPU bị "giam" trong vòng lặp I2C, không làm được việc khác. |

## 3. MÃ NGUỒN CẤU TRÚC (BARE-METAL)

```c
// [Trích i2c_mpu_debug.c] Cơ chế thoát lỗi Blocking bằng Timeout
uint8_t MPU_WriteReg(uint8_t reg, uint8_t data) {
    uint32_t t = 100000; 
    I2C1->CR1 |= I2C_CR1_START;
    // Chờ cờ khởi động SB, nếu kẹt quá lâu CPU sẽ tự thoát sau t vòng lặp
    while(!(I2C1->SR1 & I2C_SR1_SB)) { 
        if(--t==0) return 0x05; 
    }
    // ...
}
```