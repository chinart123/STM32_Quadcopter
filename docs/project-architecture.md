# PROJECT ARCHITECTURE
## Dual-Platform Quadcopter — STM32F103 + ESP32-S3

> **Đọc file này để hiểu cấu trúc dự án.**
> Trạng thái: ✅ Đang chạy | ⚠️ Một phần | ❌ Chưa làm | 🎯 Mục tiêu

---

## Triết lý kiến trúc

```
"Write once, compile everywhere"

src/drn_mpu6050.cpp
    │
    ├── ESP-IDF compiles directly
    └── Arduino IDE compiles via symlink
            Arduino_IDE/.../src/drn_mpu6050.cpp → symlink → ../../src/drn_mpu6050.cpp

drn_main_board_choose.h làm cầu nối:
    #ifdef ARDUINO
        // Arduino API: Wire.begin(), ledcWrite()...
    #else
        // ESP-IDF API: i2c_master_bus_add_device(), ledc_set_duty()...
    #endif
```

---

## 1. STM32_Quadcopter (Keil µVision 5)

**Chip:** STM32F103C8 (BluePill) | **Role:** Học sâu firmware, validate PID thuật toán

```
STM32_Quadcopter/
│
├── src/                             ✅ ACTIVE — files được build
│   ├── main.c                       ✅ Cooperative while(1) scheduler loop
│   ├── drn_main_board_choose.c      ✅ Platform abstraction (STM32 implement)
│   ├── drn_button.c                 ✅ FSM debounce, 2 nút PA0/PA1
│   ├── drn_time.c                   ✅ SysTick 1ms → DRN_Millis()
│   ├── drn_motor_pwm.c              ✅ TIM3 4kHz, 4 kênh — PA7=CH2 ⚠️ PA6 cháy
│   ├── drn_timer_pwm.c              ✅ HAL_TIM3 wrapper
│   ├── hal_timer_pwm.c              ⚠️ Cần ẩn vào drn_ layer (HAL lộ ra ngoài)
│   ├── pid_control.c                ⚠️ Struct có, chưa kết nối IMU output
│   ├── telemetry.c                  ✅ HC-05 UART CSV 10Hz → Hercules/Python
│   ├── xx_mpu_complementary_filter.c ✅ Complementary filter, alpha=0.98
│   └── xx_mpu_data_fusion.c         ✅ I2C polling MPU6050 200Hz
│
├── include/                         Headers tương ứng với src/
│   ├── drn_main_board_choose.h
│   ├── drn_button.h
│   ├── drn_motor_pwm.h
│   ├── drn_time.h
│   ├── drn_timer_pwm.h
│   ├── hal_timer_pwm.h
│   ├── pid_control.h
│   ├── telemetry.h
│   ├── xx_mpu_complementary_filter.h
│   └── xx_mpu_data_fusion.h
│
├── Drivers/                         ✅ Đã có sẵn — KHÔNG tải lại
│   ├── CMSIS/Device/ST/STM32F1xx/   ARM Cortex-M3 core
│   └── STM32F1xx_HAL_Driver/        ✅ HAL v1.8.0 — chỉ cần add include path
│       ├── Inc/                     Thêm vào Keil: Options→C/C++→Include Paths
│       └── Src/
│
├── RTE/Device/STM32F103C8/          CMSIS startup (do Keil quản lý)
│   ├── startup_stm32f10x_md.s
│   └── system_stm32f10x.c
│
├── Old_Core_Files/                  ⛔ KHÔNG BUILD — chỉ đọc tham khảo
│   ├── button.c/.h                  → đã thay bằng drn_button
│   ├── main_board_choose.c/.h       → đã thay bằng drn_main_board_choose
│   └── i2c_mpu_debug.c/.h           I2C blocking cũ (gây nghẽn CPU)
│
├── docs/1-motor.md/                 Stage logs: 10 → 10.5 Final Release
├── Logs_and_Debug/                  CSV telemetry logs thực tế
├── assets/                          Sơ đồ mạch, datasheet
├── stm32f10x.h                      ⚠️ KHÔNG DI CHUYỂN — Keil RTE quản lý
├── STM32_Quadcopter.uvprojx         Keil project file
├── super_refactor.sh                Script đổi tên convention (--dry-run trước)
└── .gitignore
```

**⚠️ Hardware warning:** PA6 (TIM3_CH1) đã bị cháy Gate. Chỉ dùng **PA7 = TIM3_CH2**.

---

## 2. ESP32-Quadcopter (ESP-IDF 5.3.1 + Arduino IDE)

**Chip:** ESP32-S3 Supermini | **Role:** Platform bay thật, dual-core 240MHz

### 2a. Cấu trúc hiện tại ✅

```
ESP32-Quadcopter/
│
├── src/                             ✅ ACTIVE — ESP-IDF compiles directly
│   ├── drn_main_board_choose.cpp    ✅ Platform abstraction (ESP32 implement)
│   ├── drn_button.cpp               ✅ FSM debounce
│   ├── drn_time.cpp                 ✅ esp_timer → DRN_Millis()
│   ├── drn_mpu6050.cpp              ✅ M1: complementary filter, 200Hz
│   ├── drn_raw_mpu6050.cpp          ✅ M1: raw data + calibration 1000 mẫu
│   ├── drn_motor_pwm.cpp            ❌ Rỗng — chưa implement
│   └── drn_timer_pwm.cpp            ⚠️ LEDC config, chưa test với motor
│
├── include/                         Headers tương ứng
│
├── main/
│   ├── main.cpp                     ✅ app_main() — cooperative while(1) loop
│   └── CMakeLists.txt
│
├── Arduino_IDE/ESP32_Quadcopter/    ⚠️ Chưa có symlinks
│   ├── ESP32_Quadcopter.ino         ⚠️ Chỉ có PWM TX/RX demo
│   ├── assets/
│   └── docs/
│
├── Old Core_Files/                  ⛔ KHÔNG BUILD
│   ├── pid_control.cpp/.h           Sẽ port vào src/ khi M1→M2
│   ├── telemetry.cpp/.h
│   ├── hal_timer_pwm.cpp/.h
│   └── xx_mpu_*.cpp
│
├── sdkconfig                        ✅ GDMA: CONFIG_GDMA_CTRL_FUNC_IN_IRAM=y
├── CMakeLists.txt
└── .gitignore
```

### 2b. Cấu trúc mục tiêu 🎯 (Symlink Architecture)

```
ESP32-Quadcopter/
│
├── src/                             Single source of truth
│   └── drn_*.cpp                    Compile được trên CẢ HAI IDE
│
├── Arduino_IDE/ESP32_Quadcopter/
│   ├── ESP32_Quadcopter.ino         setup() / loop()
│   ├── src/                         🎯 Symlinks — chưa tạo
│   │   ├── drn_mpu6050.cpp      →   ../../../src/drn_mpu6050.cpp
│   │   ├── drn_motor_pwm.cpp    →   ../../../src/drn_motor_pwm.cpp
│   │   ├── drn_button.cpp       →   ../../../src/drn_button.cpp
│   │   ├── drn_time.cpp         →   ../../../src/drn_time.cpp
│   │   └── drn_timer_pwm.cpp    →   ../../../src/drn_timer_pwm.cpp
│   └── include/                     🎯 Symlinks — chưa tạo
│       ├── drn_mpu6050.h        →   ../../../include/drn_mpu6050.h
│       ├── drn_motor_pwm.h      →   ../../../include/drn_motor_pwm.h
│       └── ... (tất cả drn_*.h)
│
└── ...
```

**Cách tạo symlinks (Windows — chạy PowerShell với quyền Admin):**
```powershell
cd D:\ESP32_Drone_Workspace\ESP32-Quadcopter\Arduino_IDE\ESP32_Quadcopter
New-Item -ItemType Directory -Force -Path src, include

# Tạo symlinks cho src/
$srcs = @("drn_mpu6050","drn_raw_mpu6050","drn_motor_pwm","drn_button","drn_time","drn_timer_pwm","drn_main_board_choose")
foreach ($f in $srcs) {
    New-Item -ItemType SymbolicLink -Path "src\$f.cpp" -Target "..\..\..\src\$f.cpp"
    New-Item -ItemType SymbolicLink -Path "include\$f.h" -Target "..\..\..\include\$f.h"
}
```

**Điều kiện để symlink hoạt động đúng:**
```cpp
// drn_main_board_choose.h — switch API theo platform
#ifdef ARDUINO
  #include <Wire.h>
  #define DRN_DELAY_MS(ms) delay(ms)
  #define DRN_MILLIS()     millis()
#else
  #include "driver/i2c_master.h"
  #include "esp_timer.h"
  #define DRN_DELAY_MS(ms) vTaskDelay(pdMS_TO_TICKS(ms))
  #define DRN_MILLIS()     ((uint32_t)(esp_timer_get_time() / 1000))
#endif
```

---

## 3. Luồng phát triển (Development Flow)

```
[STM32 — Học sâu]          [ESP32 — Bay thật]
       │                          │
A3: Test IMU+Motor         C2: Motor LEDC
       │                          │
A4: Rate PID               C4: Test IMU+Motor
       │                          │
A5: Attitude PID      →    C5: Port PID từ A4/A5
       │                          │
       └──── Verify cross-platform ────┘
              (cùng Kp/Ki/Kd, cùng kết quả)
                          │
                    B5: Arduino IDE
                    (verify lần 3)
```

---

## 4. Module dependency map

```
main.c / main.cpp
    ├── drn_main_board_choose  ← Tất cả platform API đi qua đây
    ├── drn_button             ← Dùng DRN_Millis() từ drn_time
    ├── drn_time               ← SysTick (STM32) | esp_timer (ESP32)
    ├── drn_mpu6050            ← Dùng I2C từ drn_main_board_choose
    ├── drn_motor_pwm          ← Dùng TIM3/LEDC từ drn_timer_pwm
    ├── drn_timer_pwm          ← Hardware timer abstraction
    ├── pid_control            ← Dùng output từ drn_mpu6050
    └── telemetry              ← Dùng UART từ drn_main_board_choose
```

---

## 5. Naming convention (đã chốt)

| Loại | Pattern | Ví dụ |
|------|---------|-------|
| Public API | `DRN_Module_Verb()` | `DRN_IMU_Init()`, `DRN_Motor_SetDuty()` |
| Internal static | `verb_noun()` | `static calc_filter()` |
| Struct/typedef | `DRN_Module_Name_t` | `DRN_IMU_Data_t` |
| Global variable | `drn_module_name` | `drn_btn_PA0` |
| Constants | `DRN_MODULE_NAME` | `DRN_MOTOR_MIN` |
| Debug variables | `xx_` prefix | `xx_mpu_state`, `xx_gate_state` |

---

## 6. Hardware Warnings

| ⚠️ Cảnh báo | Chi tiết |
|------------|---------|
| **PA6 cháy Gate** | TIM3_CH1 (PA6) hỏng vĩnh viễn. Chỉ dùng **PA7 = TIM3_CH2** |
| `micros()` type mismatch | STM32=`uint16` (overflow 65ms!), ESP32=`uint32` — không dùng lẫn |
| HAL include path | Thêm `Drivers/STM32F1xx_HAL_Driver/Inc` vào Keil trước khi dùng HAL |
| Symlink on Windows | Cần PowerShell Admin hoặc Developer Mode để tạo symlink |