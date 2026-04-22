# SO SÁNH ĐỘ PHỨC TẠP: NGẮT (INTERRUPT) VS DMA (STM32 VS ESP32)

Tài liệu này phân tích lý do vì sao dự án Windify chuyển hướng sang ESP32-S3 không chỉ vì hiệu năng thô, mà còn vì khả năng quản lý tài nguyên phần cứng thông minh hơn qua hệ thống GDMA và RTOS.

## 1. Độ phức tạp của phương pháp Ngắt (Interrupt)

Trong phương pháp ngắt, CPU phải trực tiếp tham gia vào việc di chuyển từng byte dữ liệu. Điều này tạo ra "Overhead" (tốn tài nguyên CPU cho việc chuyển đổi ngữ cảnh).

### Snippet: STM32F103 Interrupt (Bare-metal)
Với STM32F1, bạn phải quản lý một Máy trạng thái (State Machine) cực kỳ phức tạp ngay trong hàm phục vụ ngắt (ISR).

```c
// STM32 I2C Event Handler - Cực kỳ phức tạp và dễ lỗi nếu có nhiễu
void I2C1_EV_IRQHandler(void) {
    uint32_t event = I2C_GetLastEvent(I2C1);
    
    switch (event) {
        case I2C_EVENT_MASTER_MODE_SELECT:
            I2C_Send7bitAddress(I2C1, MPU6050_ADDR, I2C_Direction_Receiver);
            break;
        case I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED:
            // Chuẩn bị nhận dữ liệu
            break;
        case I2C_EVENT_MASTER_BYTE_RECEIVED:
            data_buffer[index++] = I2C_ReceiveData(I2C1);
            if(index == 14) {
                I2C_AcknowledgeConfig(I2C1, DISABLE);
                I2C_GenerateSTOP(I2C1, ENABLE);
                flag_read_complete = 1;
            }
            break;
        // Phải xử lý hàng chục case lỗi khác (AF, BERR, ARLO...)
    }
}
```

## 2. Độ phức tạp của phương pháp DMA

DMA giúp giải phóng CPU, nhưng cấu hình "thô" trên STM32 đòi hỏi bạn phải hiểu cực rõ về bản đồ bộ nhớ.

### Snippet: STM32F103 DMA Configuration
Bạn phải liên kết chính xác thanh ghi ngoại vi (Peripheral) với vùng RAM (Memory).

```c
// STM32 DMA Setup - Cấu hình "vật lý" thủ công
void DMA_Config(void) {
    DMA_InitTypeDef DMA_InitStructure;
    DMA_DeInit(DMA1_Channel7); // Channel cho I2C1 RX
    
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&I2C1->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)data_buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 14;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_Init(DMA1_Channel7, &DMA_InitStructure);
    
    I2C_DMACmd(I2C1, ENABLE);
    DMA_Cmd(DMA1_Channel7, ENABLE);
}
```

### Snippet: ESP32-S3 New Driver (ESP-IDF v5.x)
ESP32-S3 sử dụng **GDMA (Generic DMA)** được trừu tượng hóa cao độ. Bạn gần như không cần quan tâm DMA là gì, driver tự lo.

```cpp
// ESP32 New I2C Driver - Đơn giản, an toàn, tự động dùng GDMA
void read_mpu6050() {
    uint8_t buffer[14];
    // Driver tự cấu hình DMA và ngắt bên dưới. 
    // Task sẽ bị "Block" nhưng Core 1 vẫn rảnh để RTOS chạy task khác.
    i2c_master_transmit_receive(mpu_device_handle, &reg_addr, 1, buffer, 14, -1);
    
    // Xử lý dữ liệu ngay lập tức sau khi hàm trả về
}
```

## 3. Bảng so sánh lộ trình API & Driver

Dưới đây là sự tiến hóa của các bộ thư viện từ hai hãng. Có thể thấy ESP32 chuyển dịch rất nhanh sang hướng "Architecture-oriented" (Hướng kiến trúc).

| Năm | STM32 (STMicroelectronics) | ESP32 (Espressif Systems) |
|:---:|:---|:---|
| **Trước 2014** | **SPL (Standard Peripheral Library):** Lập trình trực tiếp thanh ghi, cực kỳ phức tạp. | *(Chưa phổ biến)* |
| **2014 - 2016** | **STM32Cube HAL:** Trừu tượng hóa cao hơn nhưng thư viện I2C đời đầu bị lỗi Blocking rất nặng. | **ESP-IDF v1.0:** Bắt đầu có driver I2C cơ bản trên nền FreeRTOS. |
| **2017 - 2022** | **LL (Low-Layer) Drivers:** ST ra mắt bộ thư viện nhẹ hơn để khắc phục sự cồng kềnh của HAL. | **Legacy I2C Driver:** Driver ổn định nhưng chưa tối ưu cho các chip đa nhân mới. |
| **2023 - Nay** | **HAL hiện đại:** Đã cải thiện xử lý DMA/Interrupt nhưng vẫn chạy trên 1 nhân đơn (Single Core). | **New I2C Master Driver (GDMA):** Tích hợp sâu DMA, hỗ trợ đa nhân hoàn hảo cho ESP32-S3. |

## 4. Tại sao vẫn chọn ESP32-S3?

1. **FPU (Floating Point Unit):** STM32F103 (Cortex-M3) phải tính số thực bằng phần mềm. [cite_start]ESP32-S3 tính bằng phần cứng, cực kỳ quan trọng cho PID [cite: 58-61, 90-92].
2. **Dual-Core:** DMA trên STM32 giải phóng CPU khỏi việc di chuyển byte, nhưng CPU vẫn phải quản lý logic bay. [cite_start]ESP32 tách biệt hoàn toàn: Core 0 lo viễn thông (RF), Core 1 lo bay (Flight loop) [cite: 93-95, 416-425].
3. **GDMA:** Hệ thống DMA của ESP32-S3 thông minh hơn, tự động xử lý việc đồng bộ dữ liệu mà không cần can thiệp thủ công vào thanh ghi như STM32.