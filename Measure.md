1. Ghi gì vào .gitignore để bỏ qua toàn bộ thư mục RTE nhỉ? Cho tôi hỏi tương lai nếu nâng cấp lên động cơ và các giao tiếp cảm biến khác thì thư mục này cũng không thay đổi phải không? Trừ khi chúng ta đổi dòng vi điều khiển thôi.
2.Ở bước 1:  Confirm Mạng lưới I2C (Biến trạng thái) 
Hiện giờ 2 biến xx_mpu_id, xx_mpu_state chưa được cập nhật nhé.
3. 
a) Ở bước 2:Confirm Gia tốc Trọng trường (Raw Data Accel)
Accel_X_RAW: dao động từ -130 đến -170, có khi vọt xuống tới -200 rồi nhảy lên lại
Accel_Y_RAW: dao động từ 0 đến -27
Accel_Z_RAW:  dao động từ 4599 đến 4650.

b) Ở Bước 3: Confirm Con Quay (Raw Data Gyro)
Gyro_X_RAW: dao động từ 60 đến 80
Y_RAW: dao động từ 35 đến 55
Z_RAW:  dao động từ -43 đến -66
Cầm mạch xoay tròn một cách bạo lực, các số này phải vọt lên hàng ngàn: Cái này đúng

c) Bước 4: Confirm Góc Bay Toán Học (Roll, Pitch - Bắt đầu Quy ước)
c.1)
Drone_IMU.Roll: Dao động từ -0.07... cho đến 0.4.... 
{
.nghiêng mũi chúi xuống từ đứng yên cho tới khi vuông góc:nhảy âm,min là  âm 90
.nghiêng mũi chúi xuống từ vuông góc cho tới khi lật ngược: nhảy âm,giảm từ âm 90 xuống âm 180, qua mức này lập tức tăng lên
//xoay cho đủ 360 độ thì về 0. 2 hành đọng bên dưới là tiếp diễn của 2 hành động trên
.nghiêng đuôi chúi lên từ đứng yên cho tới khi vuông góc:nhảy âm,min là  âm 90
.nghiêng đuôi chúi lên từ vuông góc cho tới khi lật ngược:nhảy dương,max là  dương 180
}


c.2)
Drone_IMU.Pitch: Dao động từ 1.2... cho đến 2.3...
{
.nghiêng sang trái từ đứng yên cho tới khi vuông góc:nhảy âm,min là  âm 90
.nghiêng sang trái từ vuông góc cho tới khi lật ngược : nhảy dương, tăng từ âm 90 lên 0
//xoay cho đủ 360 độ thì về 0. 2 hành đọng bên dưới là tiếp diễn của 2 hành động trên
.nghiêng sang phải từ đứng yên cho tới khi vuông góc:nhảy dương,max là  dương 90
.nghiêng sang phải từ vuông góc cho tới khi cho tới khi lật ngược : nhảy âm, giảm từ dương 90 về 0
}


Kết luận rút ra:
Accel_X_RAW là chỉ số bất ổn định nhất, có lẽ nó tùy vào vị trí đặt cảm biến trên bàn. Không bao giờ ở 0
dao động từ -170 đến -130, có khi vọt xuống tới -200 rồi nhảy lên lại
Sau khi lật xoay một hồi để đo các chỉ số khác.
dao động từ -270 đến -220, có khi vọt xuống tới -200 rồi nhảy lên lại
Sau khoảng thời gian, tổng kết lại 
-->Accel_X_RAW dao động rộng nhất từ -300 đến -200, đôi khi sẽ vọt ra ngoài dãy giá trị này nhưng không nhiều, khá là ổn định.