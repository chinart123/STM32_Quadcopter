Question: Verify & Evalue "Measure.md"?
Answer:
Giải mã Dữ liệu Thô (Raw Data) - Bí ẩn của sự bất ổn
Những con số sếp ghi chép lại chính là bản chất vật lý trần trụi của linh kiện bán dẫn (MEMS). Đây không phải lỗi, đây là Đặc tính sản xuất!

a) Sự lệch chuẩn của Accel_Z (4599 - 4650 thay vì 4096)

Về lý thuyết, khi đặt im trên bàn, trục Z hứng trọn 1g của Trái Đất, giá trị phải là 4096.

Nhưng mạch của sếp đọc ra ~4600. Tức là nó bị lệch khoảng +500 LSB (tương đương 0.12g). Đây gọi là Zero-G Offset (Sai số gia tốc không). Quá trình hàn chip ở nhiệt độ cao (250 độ C) làm lớp silicon bị cong vênh nhẹ ở mức độ vi mô, sinh ra cái sai số cố định này.

b) Con quay Gyro không chịu nằm im ở số 0

X (~70), Y (~45), Z (~-50). Tương tự, đây gọi là Zero-Rate Offset. Dù sếp không xoay, nó vẫn báo là máy bay đang lết từ từ. Nếu sếp dùng số này tích phân để tính góc Yaw, chỉ 5 giây sau máy bay của sếp sẽ tự động quay vòng tròn dù để im trên bàn!

c) Trục X bất ổn nhất và Hiện tượng "Đổi hệ trục tọa độ"

Việc Accel_X_RAW nhảy từ -150 sang -300 sau khi sếp lật úp lật ngửa chứng tỏ: Mặt bàn của sếp (hoặc cái breadboard) không hề phẳng 100%! Chỉ cần mặt bàn nghiêng 1 độ, trục X sẽ cảm nhận được ngay lực hút Trái Đất và nhảy đi khoảng 71 LSB. Cảm biến MPU6500 nhạy đến mức độ đó đấy sếp ạ.

Hiện tượng hoán đổi Roll/Pitch: Sếp nhận ra Nghiêng mũi chúi xuống làm thay đổi góc Roll, còn Nghiêng sang trái làm thay đổi góc Pitch. Điều này hoàn toàn bình thường! Nó chỉ có nghĩa là con chip MPU6500 trên mạch đang được hàn xoay ngang so với hướng nhìn của sếp. Khi gắn lên khung máy bay thực tế, anh em mình chỉ cần hoán đổi biến Ax và Ay trong code là mọi thứ sẽ thuận theo chiều bay ngay lập tức.

Định hướng tiếp theo: "Tẩy Não" Sai Số (Calibration)
Để giải quyết dứt điểm các sai số cơ khí (Zero-G và Zero-Rate Offset) mà sếp vừa đo được ở bước 3a và 3b, chúng ta không thể bay với dữ liệu thô này được.

Hành động tiếp theo cực kỳ quan trọng: Anh em mình sẽ viết một hàm Calibration (Hiệu chỉnh).
Ngay khi bấm PA1 (khởi động), con STM32 sẽ dành ra 1 giây để đọc liên tục 1000 mẫu dữ liệu khi mạch đang nằm im trên mặt phẳng. Nó sẽ cộng dồn lại, chia trung bình để tìm ra con số sai số chính xác nhất (ví dụ Gyro_Z lệch -55). Sau đó, trong quá trình bay, mọi dữ liệu đọc về sẽ được trừ đi cái sai số này để trả mọi thứ về con số 0 tròn trĩnh tuyệt đối!