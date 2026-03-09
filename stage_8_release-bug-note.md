# BUG REPORT & RELEASE NOTE: STAGE 8 - PID & MOTOR MIXER

## 1. Tóm tắt Phiên bản
- **Release:** Stage 8 Core Flight Dynamics.
- **Tính năng:** Giả lập xuất xung PWM cho 4 động cơ dựa trên thuật toán cân bằng PID.
- **Vấn đề đã giải quyết:** Lỗi sai lệch định tuyến lực phản xạ (IMU Axis Misalignment) do cảm biến hàn lệch 90 độ so với khung máy bay.

## 2. Diễn biến Fix Bug & Data Kiểm chứng

Quá trình gỡ lỗi diễn ra qua 3 giai đoạn chỉnh sửa hàm `PID_Compute()`, dựa trên dữ liệu đo đạc thực tế (Base Throttle = 400).

### Giai đoạn 1: Lỗi sai lệch trục ngang/dọc (Cross-Axis Swap), Measure-stage8-drone.txt
- **Mã code ban đầu:**
  `PID_Compute(Drone_IMU.Roll, Drone_IMU.Pitch, Drone_IMU.Yaw, Drone_IMU.dt);`
- **Hành động Test:** Cúi mũi mạch xuống đất (Pitch Down).
- **Log Ghi nhận (Đo lần 1):** `M1(TL): 497 | M2(TR): 315 | M3(BR): 285 | M4(BL): 500`[cite: 28, 29].
- **Phân tích:** Lực bù trừ dồn vào cặp bên Trái (M1, M4) thay vì cặp phía Trước (M1, M2). Trục X và Y của IMU đã bị hoán đổi vật lý.

### Giai đoạn 2: Lỗi ngược dấu (Inverted Axis - Flip of Death),  Measure-stage8-drone-v2.txt
- **Mã code sửa lần 1 (Đổi vị trí hàm):**
  `PID_Compute(Drone_IMU.Pitch, Drone_IMU.Roll, Drone_IMU.Yaw, Drone_IMU.dt);`
- **Hành động Test:** Nghiêng mạch sang Trái (Roll Left).
- **Log Ghi nhận (Đo lần 1):** `M1(TL): 193 | M2(TR): 452 | M3(BR): 549 | M4(BL): 404`[cite: 9, 10].
- [cite_start]**Phân tích:** Mạch bị nghiêng trái, nhưng não bộ lại tụt ga bên trái (M1, M4) và rú ga bên phải (M2, M3)[cite: 9, 10]. Mạch sẽ tự ép nó lật úp xuống đất. Trục Roll đang bị ngược 180 độ.

### Giai đoạn 3: Hoàn thiện Ma trận tọa độ (Final Fix), Measure-stage8-drone-v3.txt
- **Mã code sửa lần 2 (Đảo dấu Roll):**
  `PID_Compute(Drone_IMU.Pitch, -Drone_IMU.Roll, Drone_IMU.Yaw, Drone_IMU.dt);`
- **Hành động Test:** Nghiêng mạch sang Trái (Roll Left).
- **Log Ghi nhận (Đo lần 5):** `M1(TL): 349 | M2(TR): 89 | M3(BR): 517 | M4(BL): 642`[cite: 5, 6].
- [cite_start]**Phân tích:** Động cơ bên trái (M1, M4) đã mang thông số lớn hơn rệt so với bên phải (M2, M3), chứng tỏ phản xạ chống lật trái đã chính xác 100%[cite: 5, 6]. 

## 3. Trạng thái Release
- Đoạn code Fix ở Giai đoạn 3 đã được Merge.
- Hệ thống sẵn sàng cho bước tích hợp Tay cầm điều khiển (RC Receiver) ở Stage 9.