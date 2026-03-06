Question:
Giờ giải thích cho tôi xem chuyện gì đã xảy ra stage-4 bạn nhé. 
Và hướng giải quyết stage 4 đó là gì? Vì sao nó lại có tác dụng? Nó tác động vào cái gì?
Show các dòng code chủ đạo(ngắn gọn thôi) đã làm thay đổi các hàm và biến nhé
 

Answer:
Đúng là đi đến chặng cuối cùng thì mọi thứ đã trở nên viên mãn! Dựa vào cái file stage_4_mpu_6050_bug_note.md sếp vừa đưa, anh em mình sẽ đúc kết lại cái kết cục đầy bất ngờ (Plot Twist) của Stage 4 theo đúng chuẩn form báo cáo hệ thống nhé:

1. Chuyện gì đã xảy ra ở Stage 4?
Ở giai đoạn này, toàn bộ FSM và đường truyền vật lý I2C đã hoạt động hoàn hảo 100%. Lỗi 0x05 (Treo Bus) và 0x08 (Timeout) đã bị tiêu diệt hoàn toàn, trả về trạng thái 0x99 (Thành công).
Nhưng ngay tại vạch đích, sếp lại phát hiện ra 2 "hiện tượng lạ" trên màn hình debug:

Hiện tượng 1 (Ký tự rác): Biến xx_mpu_id và xx_mpu_state ngoài việc hiện số Hex/Decimal lại đẻ thêm mấy cái ký tự kì quặc như chữ 'p' hay hình thoi đen chấm hỏi ''.

Hiện tượng 2 (Nhận diện sai nhân thân): Code gọi đúng địa chỉ, nhưng con chip lại dõng dạc xưng tên nó là 0x70 (112) chứ không phải là 0x68 (104) như tất cả các tài liệu trên mạng về MPU-6050 dạy.

2. Hướng giải quyết là gì? Tác động vào đâu và vì sao có tác dụng?
Ở Stage 4 này, KHÔNG CÓ LỖI NÀO CẦN PHẢI SỬA CẢ. Đây là giai đoạn "Cập nhật Nhận thức Hệ thống".

Đối với hiện tượng "Ký tự rác" (Toolchain Quirk):

Bản chất: Sếp đang khai báo biến kiểu uchar (unsigned char). Trình gỡ lỗi (Debugger) của KeilC mặc định hiểu biến char là "Ký tự", nên nó tự động mang con số 112 hay 153 đi tra bảng mã ASCII để vẽ ra màn hình cho sếp đọc, dẫn đến việc hiện chữ 'p' hoặc ký tự lỗi.

Hướng giải quyết: Không cần sửa code. Sếp chỉ cần phớt lờ các ký tự đó, chỉ tập trung vào cột Value chứa mã Hex (0x70, 0x99).

Đối với hiện tượng "Sai ID 0x70" (Hardware Plot Twist):

Bản chất: Hãng InvenSense đã ngừng sản xuất lõi chip MPU-6050 từ lâu. Các pháp sư Trung Hoa sản xuất mạch GY-521 hiện nay đã âm thầm thay thế lõi bên trong bằng con chip đời mới hơn là MPU-6500. Theo Datasheet chính chủ của MPU-6500, thanh ghi WHO_AM_I (0x75) của nó được gán cứng giá trị mặc định là 0x70.

Hướng giải quyết: Chấp nhận danh tính mới của nó. Cập nhật lại tư duy hệ thống: Giá trị chuẩn để xác nhận cảm biến còn sống không phải là 0x68 nữa, mà là 0x70.

Tác động: Việc này không làm hỏng dự án, ngược lại còn là Tin cực vui. MPU-6500 xử lý mượt hơn, ít nhiễu hơn, và bản đồ thanh ghi (Register Map) của nó giống MPU-6050 đến 99%. Mọi thuật toán lấy mẫu, tính góc nghiêng cũ của sếp vẫn sẽ chạy ầm ầm mà không cần sửa.

3. Các dòng code chủ đạo thay đổi cục diện
Ở Stage 4, bộ mã giao tiếp tầng thấp (Hardware Layer) i2c_mpu_debug.c đã đạt đến độ hoàn mỹ và không cần thêm bất cứ bản vá lỗi nào nữa.

Sự thay đổi duy nhất nằm ở Tầng Ứng dụng (Application Layer) trong tương lai, khi sếp viết code khởi tạo Quadcopter. Sếp sẽ không còn so sánh id == 0x68 nữa, mà sẽ dùng con số mới:

// Đoạn code logic sếp sẽ dùng trong tương lai để xác nhận phần cứng
void MPU_Init_Sequence(void) {
    uint8_t state, id;
    
    // Gọi cảm biến thức dậy
    MPU_WakeUp();
    
    // Đọc danh tính
    MPU_Read_WhoAmI(&state, &id);
    
    // So sánh với danh tính của MPU-6500 thay vì MPU-6050
    if (state == 0x99 && id == 0x70) {
        // Cảm biến sống và đúng loại MPU-6500 -> Bật đèn LED xanh báo hệ thống OK
        Quadcopter_System_Ready();
    } else {
        // Lỗi phần cứng hoặc cắm nhầm cảm biến -> Bật còi tít tít báo lỗi
        System_Halt();
    }
}

Sếp bê nguyên cụm này ném vào stage_4_mpu_6050_bug_note.md rồi tiến hành lệnh Force Push (như anh đã hướng dẫn ở các bước Git trước) là sếp đã hoàn thành xuất sắc một bộ tài liệu Debug để đời rồi đấy! Đóng gói xong xuôi vụ này thì sẵn sàng chiến tới các thanh ghi Gia tốc (Accel) và Góc xoay (Gyro) chưa sếp?