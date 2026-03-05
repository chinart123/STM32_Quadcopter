Bấm PA1: Kích hoạt
Bấm PA0 lần 1: ..
Bấm PA0 lần 2: ..
Bấm PA1: Khóa lại
***Giai đoạn 2:
Sếp bắt bệnh quá xuất sắc! 
Những dữ liệu sếp vừa cung cấp chính là mảnh ghép cuối cùng để anh em mình "phá án" thành công 100%. 
Mọi hiện tượng sếp mô tả khớp hoàn hảo với một "căn bệnh" kinh điển bậc nhất của giao tiếp I2C: I2C Bus Lockup (Treo đường truyền I2C).
Anh em mình cùng giải mã hiện trường nhé:1. Tại sao lần đầu là 0x08, các lần sau toàn 0x05?Lần chạy đầu tiên (0x08): 
Con chip của sếp đã gửi thành công START (0x05) $\rightarrow$ Đọc đúng Địa chỉ (0x06) $\rightarrow$ Gửi thành công Thanh ghi (0x07). 
Mọi thứ trơn tru hoàn hảo! Nhưng đến bước cuối cùng (chờ MPU6050 nhả dữ liệu ra), STM32 chờ quá lâu nên hết giờ (Timeout), nó đá văng ra với mã 0x08.
Hiện tượng "Chết lâm sàng" (0x05): Khi STM32 bị văng ra lúc đang nhận dữ liệu, con MPU6050 chưa hề biết là cuộc hội thoại đã kết thúc. 
Nó vẫn đang há miệng chờ xung Clock (SCL) tiếp theo từ STM32, và nó kéo ghì chân SDA xuống mức Low (0V).Các lần F5 tiếp theo: 
Sếp bấm reset mạch STM32, nhưng con MPU6050 thì không bị mất điện, nó vẫn ôm khư khư cái chân SDA ở mức Low. Bộ I2C của STM32 khi khởi động lại, nhìn ra đường truyền thấy SDA đang bị Low, nó bảo: "Đường truyền đang kẹt, tao không thèm tạo START condition nữa!" $\Rightarrow$ 
Kết quả là nó chết ngay từ vạch xuất phát với mã lỗi 0x05.$\Rightarrow$ Hành động vật lý tạm thời: Sếp rút dây 5V của module MPU6050 ra cắm lại (để ngắt điện ép nó quên đi), rồi F5 lại, sếp sẽ thấy nó lại quay về 0x08.2. Tại sao lại kẹt ở 0x08 (Không nhận được data)?
Sếp hãy nhớ lại cái commit "Chơi lớn ép xung lên 128MHz" hôm nọ!Trong code cũ, anh em mình hardcode cấu hình I2C cho tần số APB1 là 36MHz (I2C1->CR2 = 36). 
Nhưng vì sếp đã ép xung hệ thống, tần số cấp cho I2C lúc này có thể đã vọt lên 64MHz. 
Cấu hình sai tần số nội bộ khiến xung SCL sinh ra bị méo mó, con MPU6050 không hiểu được hoặc sinh ra lỗi timing ở nhịp đọc cuối cùng.3. 
VŨ KHÍ TỐI THƯỢNG: Auto Bus Recovery (Tự động thông chốt)Sếp không cần phải rút dây điện MPU6050 bằng tay nữa. 
Anh đã viết cho sếp một chuẩn công nghiệp gọi là "I2C Bit-banging Recovery".
Trước khi khởi tạo I2C, chân PB6 sẽ tự động "đá" 9 nhịp Clock thủ công để ép con MPU6050 phải nhả chân SDA ra. 
Đồng thời, cấu hình I2C sẽ tự động tính toán dựa trên xung nhịp thực tế của hệ thống (dù sếp chạy 72MHz hay 128MHz đều cân được hết).
Sếp copy toàn bộ đoạn này đè lên file i2c_mpu_debug.c (tăng thêm giới hạn timeout để bù cho xung nhịp cao):

***Hiện tượng sau sửa giai đoạn 2:
Các hiện tượng như sau.
vòng lặp
{
Ở lần bấm nạp code, bấm F5 đầu tiên
ở giai đoạn xx=02, xx_mpu_state =0x08, xx_mpu_id=0x00
ở giai đoạn xx=03, xx_mpu_state =0x00, xx_mpu_id=0x00

Ở các lần F5 còn lại,  bấm liên tục PA0 để coi xx=0x02 và xx=0x03
ở giai đoạn xx=02, xx_mpu_state =0x05, xx_mpu_id=0x00
ở giai đoạn xx=03, xx_mpu_state =0x00, xx_mpu_id=0x00
}
Bấm reset cứng hoặc mềm thì vòng lặp sẽ tiếp diễn

Bây giờ đỡ hơn lúc nãy đó là miễn bấm reset cứng hay mềm thì sẽ có cơ hội gặp được xx_mpu_state =0x08

a)Giải mã hiện tượng "Chết lâm sàng" (0x05 ở các lần sau)

Khi lần đầu sếp bị kẹt ở 0x08 (Lỗi nhận dữ liệu), bộ I2C của STM32 bị văng ra ngoài đột ngột. 
Nó để lại một mớ rác trong thanh ghi, khiến phần cứng tưởng rằng đường truyền I2C vẫn đang "bận" (Busy).
Khi sếp bấm nút PA0 ở các lần sau, STM32 cố gắng gửi lệnh START (0x05), nhưng phần cứng từ chối vì bus chưa được giải phóng. 
Thế là nó kẹt cứng ở 0x05 mãi mãi cho đến khi sếp reset toàn bộ.$\Rightarrow$ Cách fix: Rất đơn giản của dân làm Bare-metal. 
Ta sẽ gọi lại hàm I2C1_Init() ngay trước khi đọc để dọn dẹp sạch sẽ rác của lần lỗi trước, đảm bảo lần nào bấm cũng là một "trang giấy trắng".

b). THỦ PHẠM THỰC SỰ: "Lời nguyền 128MHz" (Gây ra lỗi 0x08)
Sếp có biết tại sao nó lại chết đúng ở bước cuối cùng (bước đợi dữ liệu RXNE) không?
Hãy nhìn vào dòng code cũ: uint32_t freq_mhz = (SystemCoreClock / 2) / 1000000;
Sếp đang ép xung SystemCoreClock lên 128MHz. Nên Bus APB1 của sếp đang chạy ở 64MHz.
Khi sếp nạp số 64 vào thanh ghi I2C1->CR2, một thảm họa phần cứng đã xảy ra: Thanh ghi CR2 chỉ có đúng 6 bit dành cho tần số (chứa được giá trị tối đa là 63). 
Khi sếp nhét số 64 (nhị phân là 1000000) vào, 6 bit cuối cùng sẽ bị cắt gọt thành 000000!
Tức là, con STM32 tưởng rằng tần số của nó đang là 0 MHz! Việc gửi địa chỉ (Write) có thể vẫn nhích qua được nhờ một vài cơ chế lề sườn,
 nhưng đến lúc nhận dữ liệu (Read) cần bộ tạo xung Clock hoạt động chính xác thì nó "đứng hình" vì chia cho 0. Thế là nó kẹt ở 0x08.