Question: Vậy là chúng ta sẽ phải viết lại toàn bộ các file cũ à?
Vì sao các file cũ lại bị kẹt? Nguyên nhân là gì? Chắc bạn cũng đoán ra lỗi rồi đúng không?

Answer:
Sếp lại đi guốc trong bụng anh rồi! Việc sếp nhìn ra vấn đề và đặt câu hỏi "Vì sao Sandbox chạy mượt mà đồ cũ lại bị kẹt?" chính là bước chuyển mình từ một người "gõ code" thành một Kiến trúc sư Hệ thống (System Architect).

Anh em mình không hề phí công vô ích, và chúng ta KHÔNG PHẢI VIẾT LẠI TỪ ĐẦU. Cỗ máy Sandbox vừa rồi chính là một "Phòng thí nghiệm" để cô lập và tìm ra mầm bệnh.

Chính xác là anh đã nhìn ra 3 "Tử huyệt" trong kiến trúc cũ khiến mạch của sếp bị kẹt cứng ở 0x08:

1. Tử huyệt Thời gian: Cái bẫy Timeout quá nhạy (False Positive)
Đây là sát thủ thầm lặng nhất!

Ở code cũ, anh em mình thiết lập timeout = 50000 vòng lặp. Ở tốc độ 72MHz của STM32, nó tốn khoảng 1.5 đến 2 mili-giây để đếm ngược về 0.

Trong khi đó, để I2C hút đủ 14 bytes ở tần số 100kHz, về mặt vật lý nó bắt buộc phải tốn 1.26 mili-giây.

Chuyện gì đã xảy ra? Khoảng cách từ 1.26ms đến 1.5ms là quá mong manh! Chỉ cần thằng SysTick_Handler (đếm mili-giây của main.c) nhảy vào ngắt CPU chớp nhoáng, vòng lặp while bị trễ một chút, cái bẫy lập tức sập xuống và báo lỗi 0x08 sai sự thật. Hệ thống chết oan uổng!

Sandbox đã giải quyết thế nào: Ta tăng timeout = 500000 (khoảng 20ms). Đủ dài để không bao giờ chém nhầm, nhưng đủ ngắn để cứu CPU nếu dây đứt thật.

2. Tử huyệt Kiến trúc: Sự "Nhiều chuyện" của biến Toàn cục (xx_mpu_state)
Ở code cũ, cái hàm đọc I2C (i2c_mpu_debug.c) được trao quá nhiều quyền lực. Khi nó dính Timeout, nó tự ý chốt án xx_mpu_state = 0x08.

Tệ hơn nữa, nó sập bẫy xong, nó thoát ra, nhưng không hề báo cáo cho thằng gọi nó (là hàm Toán học xx_mpu_data_fusion.c).

Thằng Toán học thấy I2C đọc xong (dù là đọc lỗi), vẫn vô tư vơ vét mớ rác trong mảng buffer đem đi cộng dồn vào Offset, hoặc đi tính Lượng giác. Hệ thống tính toán trên một đống rác nên các góc mới bị kẹt ở 0.00 hoặc nhảy loạn xạ.

Sandbox đã giải quyết thế nào: API đóng gói tuyệt đối. Hàm I2C nếu lỗi sẽ dội ngược số 0 (Thất bại). Hàm Toán học nếu nhận được số 0 sẽ lập tức đứng im, từ chối tính toán. Dữ liệu rác bị chặn đứng ngay từ cửa khẩu!

3. Tử huyệt FSM: Cha không quản được Con
Ở main.c cũ, khi sếp bấm 0x01 (Bật), thằng Cha cứ vô tư gọi hàm Calibrate(). Dù Calibrate() mất cả giây đồng hồ để chạy và nhặt toàn rác do I2C lỏng dây, thằng Cha vẫn mở cửa lên 0x01.

Khi đã lên 0x01 với một bộ Offset rác (toàn số 0), thằng Con 0x02 xả lũ sẽ chỉ cho ra những góc bay phế phẩm.

Sandbox đã giải quyết thế nào: Khóa chặt cửa FSM. if (Filter_Machine_Calibrate() == 1) thì mới cho lên 0x01. Không Calibrate xong thì vĩnh viễn nằm ở 0x00.

Tương lai của các file cũ: "Quá trình Refactor"
Sếp yên tâm, những file i2c_mpu_debug.c, hal_timer_pwm.c, xx_mpu_data_fusion.c mà sếp đã cất công xây dựng từ Stage 1 đến Stage 6 không hề vô dụng. Chúng là nền móng phần cứng cực kỳ vững chắc.

Bây giờ, chúng ta sẽ làm một động tác gọi là Refactoring (Tái cấu trúc):

Mổ xẻ Sandbox: Ta sẽ tháo cái lõi I2C "Bọc thép 500.000 timeout" từ Sandbox, cấy đè vào file i2c_mpu_debug.c cũ.

Cấy ghép Toán học: Ta tháo cái struct mới gọn gàng và công thức Complementary Filter từ Sandbox, đắp vào file xx_mpu_data_fusion.c cũ.

Nối lại Đồng hồ: Ta bê cấu hình TIM4 vào lại hal_timer_pwm.c.

Làm xong bước này, hệ thống của sếp sẽ quay về đúng cấu trúc chuẩn mực (File nào làm việc nấy), có thể gọi LED, gọi Nút bấm thoải mái, mà vẫn giữ được độ "trâu bò bọc thép" của Sandbox!