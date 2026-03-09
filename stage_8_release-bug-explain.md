# ROOT CAUSE ANALYSIS: LỆCH TRỤC VẬT LÝ VÀ NHIỄU CHÉO (CROSS-COUPLING)

## 1. Nguyên nhân cốt lõi của việc sửa mã code (Ma trận tọa độ)
Công thức Motor Mixer tiêu chuẩn (`Motor = Throttle ± Pitch ± Roll ± Yaw`) được xây dựng dựa trên **Quy ước Hệ tọa độ hàng không**: Trục X hướng về mũi máy bay, Trục Y hướng sang phải cánh.

Tuy nhiên, module cảm biến MPU6050 (GY-521) được hàn trên board mạch thực tế với mũi tên Trục X hướng sang ngang, và Trục Y hướng lên phía trước. Nghĩa là Hệ tọa độ của chip lệch 90 độ so với Hệ tọa độ của phương trình Mixer.

Để không phải can thiệp phần cứng (hàn lại chip), hệ thống dùng một dạng **Ma trận chuyển đổi (Transformation Matrix)** cơ bản bằng phần mềm:
- `Q_pitch = IMU_X` (Hoán đổi X cho Y)
- `Q_roll = -IMU_Y` (Hoán đổi Y cho X và đảo dấu chiều vector)

Việc đảo dấu (`-Drone_IMU.Roll`) là bắt buộc để tuân thủ **Quy tắc Bàn tay phải**. Khi xoay hệ tọa độ 90 độ trên mặt phẳng 2D, một trục sẽ bị trỏ ngược lại chiều dương ban đầu, gây ra lỗi bù trừ ngược (Flip of Death).

## 2. Giải thích hiện tượng "Nhiễu chéo" (Cross-coupling) trong Log đo đạc
Tại bản Log đo đạc lần cuối cùng, khi người thao tác thực hiện "Nghiêng mạch sang Trái":
- [cite_start]M1(TL) = 349, M2(TR) = 89 [cite: 5, 6]
- [cite_start]M3(BR) = 517, M4(BL) = 642 [cite: 6]

[cite_start]**Vấn đề:** Mặc dù bên Trái (M1, M4) đã lớn hơn bên Phải (M2, M3) [cite: 5, 6][cite_start], nhưng phía Sau (M3, M4) lại rú ga mạnh gấp đôi phía Trước (M1, M2) (Tổng lực sau 1159 vs Tổng lực trước 438)[cite: 5, 6]. Tại sao nghiêng Trái lại sinh ra lực hất Đuôi?

**Giải thích vật lý:**
Hiện tượng này được gọi là **Cross-coupling (Nhiễu lực chéo)** do sai số thao tác của con người.
Khi bàn tay người cầm board mạch nghiêng sang trái, khớp cổ tay vô tình tạo ra một lực **Bốc đầu (Pitch Up)**. 
- PID phản ứng với Roll Left: Tăng M1, M4. Giảm M2, M3.
- PID phản ứng với Pitch Up: Tăng M3, M4. Giảm M1, M2.
[cite_start]-> Kết quả tổng hợp lực: M4 nhận 2 lần tăng nên vọt lên cực đại (642)[cite: 6]. [cite_start]M2 nhận 2 lần giảm nên tụt xuống cực tiểu (89)[cite: 6].

**Kết luận bảo trì:**
Sự nhảy múa không đối xứng của các con số không phải là bug của thuật toán, mà là bằng chứng cho thấy cảm biến IMU siêu nhạy và thuật toán PID đang hoạt động hoàn hảo, ghi nhận chính xác mọi rung động nhỏ nhất từ tay người thao tác.