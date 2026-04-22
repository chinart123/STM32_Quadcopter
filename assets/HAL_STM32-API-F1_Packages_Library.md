### 1. Nâng cấp lên HAL xịn nhất từ Codebase hiện tại

[cite_start]Dựa trên file mã nguồn bạn cung cấp, project hiện tại của bạn không phải là bare-metal thuần túy ghi trực tiếp vào thanh ghi, mà đang sử dụng bộ thư viện **SPL (Standard Peripheral Library)** cực kỳ cũ của ST (ví dụ: `stm32f10x_i2c.c` [cite: 785-786][cite_start], `misc.c` [cite: 1729-1730]). Bộ thư viện SPL này đã bị ST ngừng hỗ trợ từ lâu và không có các hàm quản lý DMA/Interrupt thông minh như HAL.

Để sử dụng các hàm HAL xịn nhất hiện tại trên Keil MDK mà không dùng CMake, bạn bắt buộc phải **loại bỏ hoàn toàn thư viện SPL cũ** và thay thế bằng mã nguồn của thư viện HAL mới. Bạn không thể mix (trộn lẫn) cả hai thư viện này trong cùng một project vì chúng sẽ xung đột định nghĩa.

### 2. Tích hợp trực tiếp HAL vào Project (Không dùng công cụ Build)

Vì bạn muốn mọi thứ hòa vào làm một và quản lý thủ công trên KeilC, đây là quy trình thao tác:

**Bước 1: Tải mã nguồn HAL gốc**
Truy cập trang chủ STMicroelectronics, tìm và tải gói **STM32CubeF1 MCU Package** (file .zip). Giải nén ra, bạn sẽ thấy thư mục `Drivers/STM32F1xx_HAL_Driver`. Đây là nơi chứa toàn bộ "tinh hoa" HAL hiện tại.

**Bước 2: Xóa thư viện SPL cũ**
[cite_start]Xóa toàn bộ các file bắt đầu bằng `stm32f10x_...` (ví dụ: `stm32f10x_i2c.c`, `stm32f10x_rcc.c`, `misc.c`) khỏi thư mục project của bạn và xóa chúng khỏi cây thư mục trong phần mềm Keil MDK[cite: 785, 1729].

**Bước 3: Đưa HAL vào project**
Copy các file `.c` (từ thư mục `Src`) và `.h` (từ thư mục `Inc`) của thư viện HAL mà bạn cần dùng (ví dụ: `stm32f1xx_hal.c`, `stm32f1xx_hal_i2c.c`, `stm32f1xx_hal_rcc.c`, `stm32f1xx_hal_gpio.c`, `stm32f1xx_hal_cortex.c` cho ngắt) vào thư mục code của bạn. Để thuận tiện nhất, hãy gom các file header (`.h`) và source (`.c`) của HAL vào chung một thư mục, giữ chúng gần nhau cho dễ quan sát và debug.

**Bước 4: Cấu hình file `stm32f1xx_hal_conf.h`**
Thư viện HAL yêu cầu một file cấu hình tổng. Bạn lấy file template `stm32f1xx_hal_conf_template.h` trong gói tải về, đổi tên thành `stm32f1xx_hal_conf.h`, đặt cạnh file `main.c`. Mở file này ra và uncomment (bỏ dấu `//`) các module bạn muốn dùng (như `#define HAL_I2C_MODULE_ENABLED`).

**Bước 5: Thêm vào Keil MDK**
Mở project Keil của bạn, click chuột phải vào thư mục Source Group, chọn *Add Existing Files...* và quét chọn toàn bộ các file `.c` của HAL vừa copy vào. Build lại project.

---

### 3. Cách STM32CubeIDE/CubeMX gọi API và Cấu trúc Lớp (Layers)

Trong một project được khởi tạo hoàn chỉnh bởi STM32CubeMX/CubeIDE, mã nguồn được chia thành **4 lớp (layers)** rõ rệt. Khi bạn gọi một hàm API "xịn", nó sẽ đi xuyên qua các lớp này:

1. **Application Layer (Lớp ứng dụng):** Nơi bạn viết logic (ví dụ: `main.c`). Ở đây, bạn chỉ gọi duy nhất 1 hàm API cấp cao:
   `HAL_I2C_Master_Receive_IT(&hi2c1, DEV_ADDR, buffer, 14);` // *Non-blocking read with Interrupt*
2. **HAL Layer (Lớp trừu tượng phần cứng):** Nằm trong file `stm32f1xx_hal_i2c.c`. Nó nhận lệnh từ App, thực hiện khóa mutex (để tránh task khác gọi trùng), bật các bit ngắt `ITBUFEN`, `ITEVTEN`, và lưu con trỏ buffer vào biến nội bộ của struct `hi2c1`. 
3. **CMSIS Layer (Lớp lõi vi điều khiển):** Nằm trong `stm32f103xb.h`. HAL sử dụng các macro ở lớp này để trỏ trực tiếp đến địa chỉ bộ nhớ vật lý. Ví dụ: `I2C1->CR2 |= I2C_CR2_ITEVTEN;`
4. **Hardware Layer (Lớp vật lý):** Chip STM32 nhận lệnh và bắt đầu kéo xung Clock trên chân SCL. Khi có data về, Hardware kích hoạt ngắt, CPU nhảy vào file `stm32f1xx_it.c`, gọi ngược lại hàm xử lý sự kiện của HAL (`HAL_I2C_EV_IRQHandler`), HAL lưu data vào buffer, và cuối cùng kích hoạt callback `HAL_I2C_MasterRxCpltCallback()` báo cho lớp Application biết là đã xong.

---

### 4. So sánh Kiến trúc (Snippets)

**🏛️ Kiến trúc STM32CubeIDE (Modern HAL + RTOS)**
Kiến trúc này phân lớp rõ ràng, trừu tượng hóa cao, file cấu hình trung tâm làm cầu nối.

```text
📦 CUBE_IDE_MODERN_PROJECT
 ┣ 📂 Application/User/Core
 ┃ ┣ 📜 main.c              // Main logic, calls HAL APIs
 ┃ ┣ 📜 stm32f1xx_it.c      // Hardware interrupt vectors route to HAL handlers
 ┃ ┗ 📜 stm32f1xx_hal_msp.c // MCU Support Package: Initializes GPIOs/Clocks for HAL
 ┣ 📂 Drivers/STM32F1xx_HAL_Driver
 ┃ ┣ 📜 stm32f1xx_hal.c         // Core HAL functions
 ┃ ┣ 📜 stm32f1xx_hal_i2c.c     // High-level I2C state machine & DMA/IT management
 ┃ ┗ 📜 stm32f1xx_hal_i2c.h     
 ┣ 📂 Drivers/CMSIS
 ┃ ┣ 📜 core_cm3.h          // ARM Cortex-M3 core definitions
 ┃ ┗ 📜 stm32f103xb.h       // Register map (e.g., #define I2C1_BASE 0x40005400)
 ┗ 📜 stm32f1xx_hal_conf.h  // Central configuration file to enable/disable HAL modules
```

**🔨 Kiến trúc hiện tại của bạn (SPL / Legacy Bare-metal)**
Dựa trên file `chinart123-stm32_quadcopter-8a5edab282632443.txt`, bạn đang sử dụng SPL trộn lẫn với logic ứng dụng.

```text
📦 YOUR_CURRENT_PROJECT
 ┣ 📂 User_Application
 ┃ ┣ 📜 main.c              // Application logic, often mixes with hardware calls
 ┃ ┣ 📜 drn_mpu6050.c       // Calls SPL functions directly (e.g., I2C_GenerateSTART(I2C1, ENABLE))
 ┃ ┗ 📜 stm32f10x_it.c      // Hardcoded manual interrupt service routines
 ┣ 📂 SPL_Libraries (Deprecated)
 ┃ ┣ 📜 stm32f10x_i2c.c     // Legacy ST Library (Source: line 785)
 ┃ ┣ 📜 stm32f10x_i2c.h     // Low-level wrappers around registers
 ┃ ┣ 📜 misc.c              // Legacy NVIC management (Source: line 1729)
 ┃ ┗ 📜 stm32f10x_rcc.c     // Legacy Clock management
 ┣ 📂 CMSIS
 ┃ ┣ 📜 system_stm32f10x.c  // System clock initialization
 ┃ ┗ 📜 stm32f10x.h         // Legacy device register map
 ┗ 📜 stm32f10x_conf.h      // Includes all SPL headers
```