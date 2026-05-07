# super_refactor_v2_journal.md — Phase 2: Naming Convention & Build Fixes

> **Trạng thái:** ✅ Đã hoàn thành Auto-Apply & Manual Fix (Build: 0 Error, 0 Warning)
> **Branch:** `refactor/naming-convention`
> **Hệ thống:** STM32_Quadcopter (Keil µVision 5)

---

## 1. Chạy Auto-Refactor Script

Script Phase 2 được thực thi để quét và đổi tên các hàm/biến sang chuẩn `DRN_`. Quá trình chạy diễn ra an toàn với tính năng tự động tạo backup.

```bash
$git checkout -b refactor/naming-convention$ bash super_refactor.sh --apply
📦 Tạo backup commit trước khi refactor...
[refactor/naming-convention 5b57def] backup: before naming convention refactor
```

### Khác biệt mã nguồn (Git Diff)
Script đã tự động nhận diện đúng word boundary và thay đổi tiền tố khởi tạo IMU trong file Data Fusion:

```diff
diff --git a/src/xx_mpu_data_fusion.c b/src/xx_mpu_data_fusion.c
index d9b5079..b2d3766 100644
--- a/src/xx_mpu_data_fusion.c
+++ b/src/xx_mpu_data_fusion.c
@@ -158,7 +158,7 @@ Nâng lên ±8g là máy bay rớt cái ạch vẫn đo được thông số.
 C
 // ... trong vòng lặp switch(xx) ...
 case 0x01: // BẤM PA1 - KHỞI ĐỘNG HỆ THỐNG
-    MPU_Fusion_Init(); // Gọi hàm cấu hình cực xịn vừa viết
+    DRN_MPU6050_Init(); // Gọi hàm cấu hình cực xịn vừa viết
      MPU_Read_WhoAmI(&xx_mpu_state, &xx_mpu_id); // Đọc ID xem còn sống không
      
      // Nếu sếp thích hiển thị trực quan, có thể chớp tắt LED xanh 1 cái ở đây để báo Init xong
```

---

## 2. Trình tự Xử lý Lỗi & Dọn dẹp thủ công (Manual Review)

Sau khi script chạy xong, tiến hành biên dịch trên KeilC và dọn dẹp các tàn dư của cấu trúc cũ. Dưới đây là trình tự xử lý để đạt được bản build thành công:

### Bước 2.1: Dọn rác khỏi giao diện KeilC
* **Vấn đề:** Các file code thế hệ 1 (`button.c`, `main_board_choose.c`, `i2c_mpu_debug.c`) đã bị chuyển sang `Old_Core_Files` trên ổ cứng, nhưng vẫn bị KeilC đem đi biên dịch gây xung đột.
* **Xử lý:** Xóa toàn bộ Group `Old_Core_Files` khỏi giao diện Project bên trái của Keil.

### Bước 2.2: Sửa lỗi Header & Xung đột UART
* **Vấn đề:** Báo lỗi `File not found` và `Symbol fputc multiply defined`.
* **Xử lý:** * Xóa bỏ hoàn toàn khối hàm `fputc` bị trùng lặp bên trong `xx_mpu_complementary_filter.c`, nhường quyền điều khiển cho module Telemetry.
    * Đổi `#include "main_board_choose.h"` thành `#include "drn_main_board_choose.h"` trong file `drn_timer_pwm.c`.
    * Xóa `#include "i2c_mpu_debug.h"` và đổi thành `#include "drn_main_board_choose.h"` trong file `xx_mpu_data_fusion.c`.

### Bước 2.3: Đồng bộ API Thời gian
* **Vấn đề:** Báo lỗi thiếu biến `global_tick` và nguy cơ tràn số của hàm `micros()`.
* **Xử lý:** * Khai báo thêm `#include "drn_time.h"` vào đầu file `xx_mpu_complementary_filter.c`.
    * Xóa bỏ các biến/hàm đếm thời gian cũ, thay thế toàn bộ bằng hàm chuẩn `DRN_Millis()`.

### Bước 2.4: Phục hồi liên kết I2C (Chiến thuật Agile)
* **Vấn đề:** Việc xóa `i2c_mpu_debug.c` làm file Data Fusion mất đi các hàm `MPU_WriteReg` và `MPU_Read_Multi`.
* **Quyết định:** Thay vì viết lại một module I2C độc lập gây mất thời gian, ưu tiên tái sử dụng Driver có sẵn để nhanh chóng tiến tới bài Test A3.
* **Xử lý:** Xóa bỏ từ khóa `static` ở các hàm giao tiếp I2C (`Core_I2C_Write`, `Core_I2C_Read`) bên trong file `xx_mpu_complementary_filter.c`. Hành động này giải phóng các hàm thành dạng Global, cho phép file `xx_mpu_data_fusion.c` mượn lại thông qua từ khóa `extern` mà không cần viết thêm logic mới.

---

## 3. Hoàn thành Milestone

Sau khi áp dụng các chỉnh sửa trên, project đã Build thành công với `0 Error(s), 0 Warning(s)`. Mã nguồn được chốt hạ bằng commit:

```text
chore: success build-v1 after refactor

* delete Old-core-files, delete static keyword in xx_mpu_complementary_filter.c
* add "#include drn_time.h"
* remove "i2c_mpu_debug.h" + add "#include drn_main_board_choose.h" in xx_mpu_data_fusion.c
```

## ⚠️ Cảnh báo chuẩn bị cho Bước A3 (Hardware Test)
* **Trạng thái PA6:** Chân PA6 (TIM3_CH1) đã bị cháy Gate từ đợt kiểm thử trước.
* **Hành động bắt buộc:** Trước khi cắm nguồn cho 4 động cơ vào chiều nay, phải gom logic `DRN_Timer_PWM_Init` vào `DRN_Motor_PWM_Init` và đảm bảo tuyệt đối chỉ kích hoạt kênh cấp xung PWM trên chân **PA7**.