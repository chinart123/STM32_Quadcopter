# CONTEXT.md — Drone Project AI Context File
## Đọc file này đầu tiên. Cập nhật mỗi khi có thay đổi lớn.

---

## 1. Project Snapshot

**Mục tiêu:** Indoor autonomous hover quadcopter — tự giữ vị trí không cần GPS.
**Người thực hiện:** 2 người — Owner (STM32 + ESP32 thuật toán sâu) + Friend (ESP32 Arduino IDE).
**Ngôn ngữ:** C (STM32), C++ (ESP32 ESP-IDF), Arduino C++ (ESP32 Arduino IDE).

### Hardware BOM
| Linh kiện | Model | Số lượng | Vai trò |
|-----------|-------|----------|---------|
| MCU chính | ESP32-S3 Supermini | 1 | Bay thật, dual-core 240MHz |
| MCU học | STM32F103C8 (BluePill) | 1 | Học sâu firmware, validate PID |
| Motor | Coreless 8520 (8.5×20mm) | 4 | Micro drone |
| Motor driver | MOSFET A03400A | 4 | SOT-23, 5.8A, gate PWM |
| IMU | MPU6050 (GY-521) | 1 | Gia tốc + gyro |
| ToF sensor | VL535L1X | 1 | Độ cao laser (sau PID ổn) |
| Optical Flow | PMW3901 | 1 | Giữ vị trí ngang (sau ToF) |
| Power | LDO ME6211C33 | 1 | 3.3V ổn định |
| Battery | 1S LiPo 500-650mAh | 1 | 30C min |
| Protection | Diode 1N4148W | 4 | Chống ngược từ motor |
| Filter | 220µF + 10k/20k | 1+2 | Lọc nguồn |
| RC remote | TBS Tango 2 Pro | 1 | CRSF protocol (chưa tích hợp) |

---

## 2. Cấu trúc Repository

### STM32_Quadcopter (Keil µVision)
```
STM32_Quadcopter/
├── src/                        ← ACTIVE SOURCE CODE
│   ├── main.c                  ← Entry point, cooperative while(1) loop
│   ├── drn_button.c
│   ├── drn_main_board_choose.c ← Platform abstraction layer
│   ├── drn_motor_pwm.c         ← TIM3 4kHz, PA7=CH2 (PA6 đã cháy Gate)
│   ├── drn_time.c              ← SysTick ms counter
│   ├── drn_timer_pwm.c
│   ├── hal_timer_pwm.c         ← HAL wrapper (cần cleanup)
│   ├── pid_control.c           ← PID struct có, chưa kết hợp IMU
│   ├── telemetry.c             ← HC-05 UART CSV 10Hz
│   ├── xx_mpu_complementary_filter.c
│   └── xx_mpu_data_fusion.c    ← I2C polling MPU6050
├── include/                    ← Headers tương ứng
├── Drivers/
│   ├── CMSIS/                  ← Core ARM
│   └── STM32F1xx_HAL_Driver/   ← ✅ HAL ĐÃ CÓ SẴN (Inc/ + Src/)
├── Old_Core_Files/             ← Code cũ, không build, giữ tham khảo
│   ├── button.c/.h             → đã port sang drn_button
│   ├── main_board_choose.c/.h  → đã port sang drn_main_board_choose
│   └── i2c_mpu_debug.c/.h
├── docs/1-motor.md/            ← Stage logs (10 → 10.5 Final Release)
├── Logs_and_Debug/             ← CSV telemetry logs thực tế
├── RTE/Device/STM32F103C8/     ← CMSIS startup files
└── STM32_Quadcopter.uvprojx    ← Keil project file
```

**Trạng thái STM32:**
- Stage 10.5 Final Release: motor TIM3 4kHz, 4 kênh, **PA7=CH2** (PA6 đã bị cháy Gate — KHÔNG dùng lại)
- MPU6050 polling 200Hz qua I2C bare-metal
- `PID_Roll/Pitch/Yaw` struct có, **chưa kết hợp với IMU output**
- Telemetry HC-05 → Hercules (PC) + Python log + Android app
- HAL driver đã có trong `Drivers/` — **chưa được dùng trong code**

### ESP32-Quadcopter (ESP-IDF + Arduino IDE)

#### Cấu trúc hiện tại (thực tế)
```
ESP32-Quadcopter/
├── src/                          ← ACTIVE SOURCE CODE (ESP-IDF)
│   ├── drn_button.cpp
│   ├── drn_main_board_choose.cpp ← Platform abstraction
│   ├── drn_motor_pwm.cpp         ← ❌ Chưa implement
│   ├── drn_mpu6050.cpp           ← ✅ M1 xong: filter + 200Hz task
│   ├── drn_raw_mpu6050.cpp       ← ✅ M1 xong: raw + calibration 1000 mẫu
│   ├── drn_time.cpp
│   └── drn_timer_pwm.cpp
├── include/                      ← Headers tương ứng
├── main/main.cpp                 ← app_main(), cooperative while(1) loop
├── Arduino_IDE/ESP32_Quadcopter/
│   └── ESP32_Quadcopter.ino      ← Arduino version (chỉ có PWM demo, chưa có symlinks)
├── Old Core_Files/               ← Deprecated, không build
├── sdkconfig                     ← GDMA enabled: CONFIG_GDMA_CTRL_FUNC_IN_IRAM=y
└── CMakeLists.txt
```

#### Cấu trúc mục tiêu (symlink architecture — chưa implement)
```
ESP32-Quadcopter/
├── src/                          ← Single source of truth
│   ├── drn_mpu6050.cpp           ← Dùng chung ESP-IDF + Arduino
│   ├── drn_motor_pwm.cpp
│   └── ...
├── include/
│   └── drn_main_board_choose.h   ← #ifdef ARDUINO / #else ESP-IDF #endif
├── Arduino_IDE/ESP32_Quadcopter/
│   ├── ESP32_Quadcopter.ino
│   ├── src/                      ← Symlinks trỏ về ../../src/
│   │   ├── drn_mpu6050.cpp  →  symlink → ../../../src/drn_mpu6050.cpp
│   │   ├── drn_motor_pwm.cpp → symlink → ../../../src/drn_motor_pwm.cpp
│   │   └── ... (tất cả drn_*.cpp)
│   └── include/                  ← Symlinks trỏ về ../../include/
│       ├── drn_mpu6050.h    →  symlink → ../../../include/drn_mpu6050.h
│       └── ... (tất cả drn_*.h)
└── main/main.cpp
```

**Tại sao symlink quan trọng:**
Cùng 1 file `drn_mpu6050.cpp` compile được trên cả ESP-IDF lẫn Arduino IDE. `drn_main_board_choose.h` dùng `#ifdef ARDUINO` để switch API. Không cần viết 2 bản code — fix bug 1 chỗ, cả hai IDE đều được fix.

**Cách tạo symlink trên Windows (Git Bash / PowerShell admin):**
```bash
# Trong Git Bash (chạy với quyền admin):
cd Arduino_IDE/ESP32_Quadcopter
mkdir -p src include
ln -s ../../../src/drn_mpu6050.cpp src/drn_mpu6050.cpp
ln -s ../../../src/drn_motor_pwm.cpp src/drn_motor_pwm.cpp
# ... lặp lại cho tất cả drn_*.cpp và drn_*.h

# Hoặc PowerShell (admin):
New-Item -ItemType SymbolicLink -Path "src\drn_mpu6050.cpp" -Target "..\..\..\src\drn_mpu6050.cpp"
```

**Trạng thái ESP32:**
- M1 ✅: `drn_mpu6050` + `drn_raw_mpu6050` chạy 200Hz, complementary filter
- M2 ❌: dual-core FreeRTOS (`xTaskCreatePinnedToCore`)
- Motor ❌: `drn_motor_pwm.cpp` rỗng
- Symlink architecture ❌: chưa tạo, Arduino_IDE chỉ có `.ino` demo
- GDMA config sẵn trong `sdkconfig`, chưa dùng trong driver

---

## 3. Kiến trúc đã chốt

### Naming Convention
```
Public API:     DRN_Module_Verb()      → DRN_IMU_Init(), DRN_Motor_SetDuty()
Internal:       static verb_noun()     → static calc_filter(), static set_channel()
Struct:         DRN_Module_Name_t      → DRN_IMU_Data_t, DRN_PID_Ctrl_t
Global var:     drn_module_name        → drn_imu_data, drn_btn_PA0
Constants:      DRN_MODULE_NAME        → DRN_MOTOR_MIN, DRN_PWM_FREQ
Debug vars:     xx_ prefix             → xx_mpu_state, xx_gate_state (convention của owner)
```

### Naming Conflicts đã quyết định
| Xung đột | Quyết định |
|----------|-----------|
| `DRN_Button_Context` vs `Button_Context` | `DRN_Button_Context` thắng |
| `PID_Controller_t` vs `PID_t` | `PID_Controller_t` thắng |
| `DRN_Button_FSM_Process` vs `button_fsm_process` | `DRN_Button_FSM_Process` thắng |
| `drn_main_board_choose_Init` vs `main_board_choose_Init` | `drn_main_board_choose_Init` thắng |
| `MPU_Fusion_*` vs `DRN_MPU6050_*` | `DRN_MPU6050_*` thắng |
| `pid_compute()` (ESP32 notes) | Rename thành `Calculate_Single_PID()` |

### Scheduler Pattern (Cooperative — cả hai chip)
```c
// Pattern đang dùng — KHÔNG dùng xTaskCreate ở M1
while(1) {
    uint32_t t = DRN_Millis(); // STM32: SysTick | ESP32: esp_timer_get_time()/1000
    DRN_Button_State_Hardware_Scan();
    DRN_Button_FSM_Process(&drn_btn_PA0, t);
    DRN_MPU6050_Run_Task(t);   // tự skip nếu < 5ms
    DRN_Motor_Run_Task(t);     // tự skip nếu chưa cần
    DRN_Telemetry_Run_Task(t); // tự skip nếu < 100ms
}
// M2: wrap while(1) này vào xTaskCreatePinnedToCore(..., Core1)
```

### Platform Abstraction (drn_main_board_choose)
```
STM32F103 implement:   HAL_Delay → drn_main_board_choose_Delay_ms
                       DRN_Millis() → SysTick counter
ESP32-S3 implement:    vTaskDelay → drn_main_board_choose_Delay_ms  
                       DRN_Millis() → esp_timer_get_time()/1000
```

---

## 4. Trạng thái từng module (cập nhật mỗi milestone)

| Module | STM32 (Keil) | ESP32 (ESP-IDF) | ESP32 (Arduino) |
|--------|-------------|----------------|----------------|
| MPU6050 đọc raw | ✅ polling 200Hz | ✅ `drn_raw_mpu6050` | ❌ chưa có |
| Complementary filter | ✅ `xx_mpu_complementary_filter` | ✅ `drn_mpu6050` | ❌ |
| IMU calibration | ❌ | ✅ 1000 mẫu offset | ❌ |
| PWM motor output | ✅ TIM3 4kHz, PA7, 4 kênh | ❌ `drn_motor_pwm` rỗng | ❌ |
| Test IMU + Motor (không PID) | ❌ **Bước tiếp theo A3** | ❌ B4/C4 | ❌ |
| PID Rate loop | ⚠️ struct có, chưa kết nối | ❌ | ❌ |
| PID Attitude loop | ❌ | ❌ | ❌ |
| Telemetry | ✅ HC-05 CSV 10Hz | ❌ (USB CDC planned) | ❌ |
| RC nhận tín hiệu | ❌ | ✅ ISR PWM (cần fix 3 lỗi) | ✅ ISR có |
| CRSF / TBS Tango 2 | ❌ | ❌ chờ hardware | ❌ |
| I2C Interrupt | ❌ | ❌ | ❌ |
| I2C DMA/GDMA | ❌ | ❌ (config sẵn sdkconfig) | ❌ |
| VL535L1X ToF | ❌ | ❌ | ❌ |
| PMW3901 Flow | ❌ | ❌ | ❌ |

---

## 5. Bước tiếp theo (theo thứ tự ưu tiên)

```
1. [STM32] A3 — Test IMU+Motor không PID
   → Đọc xx_mpu_data_fusion.c lấy pitch/roll
   → Dùng DRN_Timer_PWM_SetDuty() cho motor
   → Test trên bập bênh, tháo cánh quạt
   → Log qua telemetry để verify chiều đúng

2. [STM32] A4 — Kết nối PID với IMU
   → pid_control.c đã có PID_Roll/Pitch/Yaw
   → Chỉ cần feed output từ xx_mpu vào PID_Compute()
   → Tune Rate PID trước (Kp=0.5, I=0, D=0)

3. [ESP32] C2 — Motor LEDC ESP-IDF
   → implement drn_motor_pwm.cpp (hiện rỗng)
   → 4 channel LEDC 50Hz 14-bit
   → Test bằng cách port code từ STM32

4. [ESP32] C4 — Test IMU+Motor ESP32
   → Sau khi C2 xong
```

---

## 6. Cảnh báo phần cứng

| Vấn đề | Mô tả |
|--------|-------|
| **PA6 cháy Gate** | STM32 TIM3_CH1 (PA6) đã hỏng. Dùng **PA7 = TIM3_CH2**. Không assign pin PA6 cho bất kỳ chức năng nào |
| `micros()` overflow | STM32 trả `uint16` (overflow 65ms!), ESP32 trả `uint32`. Không dùng lẫn |
| HAL đã có sẵn | `Drivers/STM32F1xx_HAL_Driver/` đã có — không cần tải lại. Chỉ cần thêm include path trong Keil |
| Old_Core_Files | Không được include vào build. Chỉ đọc tham khảo |
| `drn_mpu_data` vs `Drone_IMU` | ESP32 có 2 global instance — cần merge về 1 (`Drone_IMU`) |

---

## 7. IDE và toolchain

| Platform | IDE | Compiler | Flash tool |
|----------|-----|----------|-----------|
| STM32F103 | Keil µVision 5 | ARM Compiler 5/6 | ST-Link V2 |
| ESP32-S3 | VS Code + ESP-IDF 5.3.1 | xtensa-esp32s3-elf-gcc | USB CDC |
| ESP32-S3 | Arduino IDE 2.x | arduino-esp32 | USB CDC |

---

## 8. File quan trọng cần đọc trước khi sửa code

| Mục đích | File |
|----------|------|
| Hiểu toàn bộ lộ trình | `docs/current_and_future-knowledge-updated-v5.md` |
| Stage log motor STM32 | `docs/1-motor.md/Stage_10.5-Final-Release-Motor*.md` |
| M1 milestone report | `docs/M1-docs/M1-report-esp32-v2.md` |
| M2 plan | `docs/M1-docs/M2-refactor-code-base-architecture-plan.md` |
| Complementary filter simulator | `assets/imu_simulator_complementary_filter_with-dashboard-v2.html` |
| Symbol table & naming | `assets/symbol_table_v2_with_ide.html` |