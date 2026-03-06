Tuyệt vời sếp! Nhìn vào cái file stage_3_mpu_6050_bug_note.md sếp soạn thì nội dung đã đi thẳng vào trọng tâm rồi. Để sếp có tài liệu cực chuẩn, anh sẽ hệ thống hóa lại "bức tranh" của Stage 3 theo đúng format mà chúng ta đã làm ở Stage 2 nhé:
1. Chuyện gì đã xảy ra ở Stage 3?
Sau khi vượt qua Stage 2, hệ thống FSM đã chạy cực kỳ trơn tru. Sếp bấm nút vòng lặp liên tục 0x02 $\leftrightarrow$ 0x03 mà không bao giờ bị văng lỗi "chết lâm sàng" 0x05 nữa. Nghĩa là đường truyền I2C vật lý đã được thông suốt hoàn toàn.
Hiện tượng duy nhất còn sót lại: Mã lỗi 0x08 (Quá thời gian chờ nhận dữ liệu).

Bản chất vấn đề: Ở giai đoạn này, STM32 đã gõ cửa đúng địa chỉ (qua 0x06), đã chọn đúng thanh ghi (qua 0x07). MPU-6050 đã chốt data vào bộ đệm, há miệng chờ STM32 bơm xung nhịp Clock (SCL) để đẩy data sang. Nhưng STM32 lại im re, không chịu tạo xung Clock. Chờ lâu quá Timeout, hàm tự động đá văng ra và báo 0x08.

2. Hướng giải quyết là gì? Tác động vào đâu và vì sao có tác dụng?
Hướng giải quyết: Sắp xếp lại "thứ tự hành hình" (trình tự xóa cờ và cài cờ) ở đúng bước đọc 1-Byte cuối cùng.

Tác động vào: Các thanh ghi điều khiển I2C1->CR1 (chứa cờ ACK, STOP) và thanh ghi trạng thái I2C1->SR1, I2C1->SR2 (chứa cờ ADDR).

Vì sao có tác dụng (Bí ẩn Silicon Bug): Khác với các dòng chip STM32 đời mới, riêng họ vi điều khiển STM32F1 có một lỗi thiết kế phần cứng (Silicon Bug) từ nhà máy, được hãng ST ghi chú rất rõ trong tài liệu Errata Sheet.
Khi thực hiện thao tác chỉ đọc duy nhất 1 Byte, cơ chế phần cứng của STM32F1 yêu cầu một "nghi thức" cực kỳ ngặt nghèo: Nó bắt phần mềm phải Xóa cờ ADDR trước, rồi Mới được phép cài cờ STOP.
Ở code cũ của Stage 1 và Stage 2, sếp cài cờ STOP trước khi xóa cờ ADDR. Thao tác ngược này khiến bộ máy trạng thái I2C bên trong lõi STM32F1 bị "tẩu hỏa nhập ma", từ chối tạo ra 8 xung Clock cuối cùng để hút dữ liệu về!

3. Các dòng code chủ đạo thay đổi cục diện
Sếp nhìn lại đoạn cuối của hàm MPU_Read_WhoAmI để thấy sự lột xác:

❌ Đoạn code cũ gây lỗi 0x08 (Tử huyệt của STM32F1):
// LỖI: Cài cờ STOP trước khi xóa cờ ADDR khiến phần cứng bị "lú"
I2C1->CR1 &= ~I2C_CR1_ACK;        // Tắt ACK
I2C1->CR1 |= I2C_CR1_STOP;        // Cài cờ STOP (SAI THỨ TỰ)
(void)I2C1->SR1; (void)I2C1->SR2; // Đọc để xóa cờ ADDR (BỊ TRỄ NHỊP)

✅ Đoạn code mới giải quyết triệt để (Nghi thức 1-Byte Read):
// ĐÚNG: Xóa cờ ADDR trước, cài cờ STOP ngay lập tức theo sau
I2C1->CR1 &= ~I2C_CR1_ACK;        // 1. Tắt ACK TRƯỚC TIÊN
(void)I2C1->SR1; (void)I2C1->SR2; // 2. Ép đọc SR1, SR2 để XÓA CỜ ADDR
I2C1->CR1 |= I2C_CR1_STOP;        // 3. Cài cờ STOP NGAY SAU KHI ADDR ĐÃ XÓA

Chỉ cần đổi chỗ 2 dòng code mỏng manh đó, dòng chảy thời gian của I2C lập tức được khai thông. STM32 bơm đủ 8 xung nhịp, hút thành công chữ ký của MPU về. Và phần thưởng xứng đáng cho sếp chính là con số 0x99 (PASS) và 0x70 (ID MPU-6500) chễm chệ xuất hiện trên màn hình Watch (như trong bức ảnh image_3d8c2b.png sếp đính kèm đấy).

Sếp đắp phần giải thích này vào file .md của Stage 3 rồi thực hiện lệnh git commit --amend là chốt sổ được series "Du hành thời gian" cực xịn này rồi! Xong việc đẩy mạnh (Force Push) lên Git là anh em mình có thể kê cao gối ngủ, tự tin giao tiếp với bất kỳ con cảm biến I2C nào trên đời!