#include "xx_mpu_complementary_filter.h"
#include <math.h>

DMPU_Motion_t_complementary_filter Filter_IMU;
extern volatile unsigned int global_tick; // Lấy đồng hồ mili-giây từ main.c sang

// =====================================================================
// KHỐI 1: PHẦN CỨNG ĐỘC LẬP (I2C, TIMER 4, UART 1)
// =====================================================================
static void Core_Timer4_Init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
    TIM4->PSC = 71; 
    TIM4->ARR = 0xFFFF;
    TIM4->EGR |= TIM_EGR_UG;
    TIM4->CR1 |= TIM_CR1_CEN;
}

static uint16_t Core_Micros(void) { return TIM4->CNT; }

void Filter_Machine_UART_Init(void) {
    // Bật chân PA9 (TX) truyền dữ liệu lên máy tính
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPAEN;
    GPIOA->CRH &= ~(0xFF << 4); 
    GPIOA->CRH |= (0x0B << 4);  // PA9 TX AF-PP 50MHz
    GPIOA->CRH |= (0x04 << 8);  // PA10 RX Floating Input
    
    // Ép tốc độ Baudrate = 115200 (Clock 72MHz)
    USART1->BRR = 0x271;
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

// Ghi đè hàm fputc để hàm printf có thể đẩy chữ ra chân PA9
int fputc(int ch, FILE *f) {
    while (!(USART1->SR & USART_SR_TXE));
    USART1->DR = (ch & 0xFF);
    return ch;
}

static void Core_I2C1_Recovery(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    GPIOB->CRL &= ~(0xFF << 24); GPIOB->CRL |= (0x77 << 24); 
    GPIOB->ODR |= (1 << 7); 
    for(int i=0; i<9; i++) {
        GPIOB->ODR |= (1 << 6); for(volatile int j=0; j<2000; j++); 
        GPIOB->ODR &= ~(1 << 6); for(volatile int j=0; j<2000; j++);
    }
    GPIOB->ODR &= ~(1 << 7); for(volatile int j=0; j<2000; j++);
    GPIOB->ODR |= (1 << 6);  for(volatile int j=0; j<2000; j++);
    GPIOB->ODR |= (1 << 7);  for(volatile int j=0; j<2000; j++);
}

static void Core_I2C1_Init(void) {
    Core_I2C1_Recovery();
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; 
    GPIOB->CRL &= ~(0xFF << 24); GPIOB->CRL |= (0xFF << 24);  
    RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST; RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;
    I2C1->CR2 = 36; I2C1->CCR = 180; I2C1->TRISE = 37;
    I2C1->CR1 |= I2C_CR1_PE | I2C_CR1_ACK; 
}

static uint8_t Core_I2C_Read(uint8_t reg, uint8_t *data, uint8_t size) {
    uint32_t t;
    t = 500000; while(I2C1->SR2 & I2C_SR2_BUSY) { if(--t==0) goto err; }
    I2C1->CR1 |= I2C_CR1_START;
    t = 500000; while(!(I2C1->SR1 & I2C_SR1_SB)) { if(--t==0) goto err; }
    I2C1->DR = 0x68 << 1; 
    t = 500000; while(!(I2C1->SR1 & I2C_SR1_ADDR)) { if(--t==0) goto err; }
    (void)I2C1->SR2; 
    I2C1->DR = reg;
    t = 500000; while(!(I2C1->SR1 & I2C_SR1_TXE)) { if(--t==0) goto err; }
    I2C1->CR1 |= I2C_CR1_START;
    t = 500000; while(!(I2C1->SR1 & I2C_SR1_SB)) { if(--t==0) goto err; }
    I2C1->DR = (0x68 << 1) | 1; 
    t = 500000; while(!(I2C1->SR1 & I2C_SR1_ADDR)) { if(--t==0) goto err; }
    (void)I2C1->SR2; 
    I2C1->CR1 |= I2C_CR1_ACK;
    for(uint8_t i = 0; i < size; i++) {
        if(i == size - 1) { I2C1->CR1 &= ~I2C_CR1_ACK; I2C1->CR1 |= I2C_CR1_STOP; }
        t = 500000; while(!(I2C1->SR1 & I2C_SR1_RXNE)) { if(--t==0) goto err; }
        data[i] = I2C1->DR;
    }
    return 1;
err:
    Core_I2C1_Recovery(); Core_I2C1_Init();
    return 0; 
}

static void Core_I2C_Write(uint8_t reg, uint8_t data) {
    I2C1->CR1 |= I2C_CR1_START; while(!(I2C1->SR1 & I2C_SR1_SB));
    I2C1->DR = 0xD0; while(!(I2C1->SR1 & I2C_SR1_ADDR)); (void)I2C1->SR1; (void)I2C1->SR2;
    I2C1->DR = reg;  while(!(I2C1->SR1 & I2C_SR1_TXE));
    I2C1->DR = data; while(!(I2C1->SR1 & I2C_SR1_BTF));
    I2C1->CR1 |= I2C_CR1_STOP;
}

// =====================================================================
// KHỐI 2: LOGIC MÁY LỌC VÀ ĐO LƯU LƯỢNG
// =====================================================================
void Filter_Machine_Init(void) {
    Core_I2C1_Init(); Core_Timer4_Init();
    Core_I2C_Write(0x6B, 0x00); for(volatile int i=0; i<72000; i++); 
    Core_I2C_Write(0x1A, 0x03);
    Core_I2C_Write(0x1B, 0x08);
    Core_I2C_Write(0x1C, 0x10);
}

uint8_t Filter_Machine_Calibrate(void) {
    int32_t s_ax=0, s_ay=0, s_az=0, s_gx=0, s_gy=0, s_gz=0;
    uint8_t buf[14]; 
    for (int i = 0; i < 1000; i++) {
        if (Core_I2C_Read(0x3B, buf, 14) == 0) return 0; 
        s_ax += (int16_t)((buf[0] << 8) | buf[1]); s_ay += (int16_t)((buf[2] << 8) | buf[3]);
        s_az += (int16_t)((buf[4] << 8) | buf[5]); s_gx += (int16_t)((buf[8] << 8) | buf[9]);
        s_gy += (int16_t)((buf[10] << 8) | buf[11]); s_gz += (int16_t)((buf[12] << 8) | buf[13]);
        for(volatile int j=0; j<7200; j++); 
    }

    Filter_IMU.Accel_X_Offset = s_ax/1000; Filter_IMU.Accel_Y_Offset = s_ay/1000; Filter_IMU.Accel_Z_Offset = (s_az/1000) - 4096;
    Filter_IMU.Gyro_X_Offset = s_gx/1000; Filter_IMU.Gyro_Y_Offset = s_gy/1000; Filter_IMU.Gyro_Z_Offset = s_gz/1000;
    
    Filter_IMU.last_time = Core_Micros();
    return 1;
}

uint8_t Filter_Machine_Run(void) {
    uint8_t buf[14];
    
    // 1. Hút I2C
    if (Core_I2C_Read(0x3B, buf, 14) == 0) return 0;
    
    // 2. TÍNH TOÁN LƯU LƯỢNG DATA (BYTES PER SECOND)
    static uint32_t byte_counter = 0;
    static uint32_t last_sec_tick = 0;
    
    byte_counter += 14; // Hút thành công thì được cộng thêm 14 bytes
    
    if (global_tick - last_sec_tick >= 1000) { // Đã trôi qua 1000ms (1 giây)
        Filter_IMU.Bytes_Per_Second = byte_counter; // Chốt sổ báo cáo cho sếp
        byte_counter = 0;                           // Reset để đếm lại giây mới
        last_sec_tick = global_tick;
    }
    
    // 3. Gán và Xử lý Toán học (Đã khôi phục biến Offset cho sếp)
    Filter_IMU.Accel_X_RAW = (int16_t)((buf[0] << 8) | buf[1]);
    Filter_IMU.Accel_Y_RAW = (int16_t)((buf[2] << 8) | buf[3]);
    Filter_IMU.Accel_Z_RAW = (int16_t)((buf[4] << 8) | buf[5]);
    Filter_IMU.Gyro_X_RAW  = (int16_t)((buf[8] << 8) | buf[9]);
    Filter_IMU.Gyro_Y_RAW  = (int16_t)((buf[10] << 8) | buf[11]);
    Filter_IMU.Gyro_Z_RAW  = (int16_t)((buf[12] << 8) | buf[13]);
    
    Filter_IMU.Ax = (float)(Filter_IMU.Accel_X_RAW - Filter_IMU.Accel_X_Offset) / 4096.0f;
    Filter_IMU.Ay = (float)(Filter_IMU.Accel_Y_RAW - Filter_IMU.Accel_Y_Offset) / 4096.0f;
    Filter_IMU.Az = (float)(Filter_IMU.Accel_Z_RAW - Filter_IMU.Accel_Z_Offset) / 4096.0f;
    Filter_IMU.Gx = (float)(Filter_IMU.Gyro_X_RAW - Filter_IMU.Gyro_X_Offset) / 65.5f;
    Filter_IMU.Gy = (float)(Filter_IMU.Gyro_Y_RAW - Filter_IMU.Gyro_Y_Offset) / 65.5f;
    Filter_IMU.Gz = (float)(Filter_IMU.Gyro_Z_RAW - Filter_IMU.Gyro_Z_Offset) / 65.5f;
    
    uint16_t now = Core_Micros();
    Filter_IMU.dt = (float)((uint16_t)(now - Filter_IMU.last_time)) / 1000000.0f;
    Filter_IMU.last_time = now;
    
    float Accel_Roll  = atan2(Filter_IMU.Ay, Filter_IMU.Az) * 57.2957795f;
    float Accel_Pitch = atan2(-Filter_IMU.Ax, sqrt(Filter_IMU.Ay*Filter_IMU.Ay + Filter_IMU.Az*Filter_IMU.Az)) * 57.2957795f;
    
    Filter_IMU.Roll  = 0.98f * (Filter_IMU.Roll  + Filter_IMU.Gx * Filter_IMU.dt) + 0.02f * Accel_Roll;
    Filter_IMU.Pitch = 0.98f * (Filter_IMU.Pitch + Filter_IMU.Gy * Filter_IMU.dt) + 0.02f * Accel_Pitch;
    Filter_IMU.Yaw   = Filter_IMU.Yaw + Filter_IMU.Gz * Filter_IMU.dt;
    
    return 1;
}