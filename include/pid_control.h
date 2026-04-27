#ifndef PID_CONTROL_H
#define PID_CONTROL_H

#include "stm32f10x.h"

// Cấu trúc chuẩn của một Bộ điều khiển PID
typedef struct {
    float Kp;         // Lực nhún lò xo (Phản xạ tức thời)
    float Ki;         // Lực bù trừ dồn tích (Chống lệch gió)
    float Kd;         // Lực phanh giảm xóc (Chống rung lắc)
    
    float setpoint;   // Góc bay mong muốn (Do tay cầm điều khiển truyền xuống)
    float error;      // Sai số hiện tại
    float integral;   // Kho tích phân
    float prev_error; // Sai số vòng lặp trước (Để tính đạo hàm)
    
    float out;        // Lực bù trả về (Nạp vào Motor)
    float out_max;    // Giới hạn an toàn (Tránh động cơ rú ga tối đa)
} PID_Controller_t;

// Khai báo 3 trục độc lập
extern PID_Controller_t PID_Roll;
extern PID_Controller_t PID_Pitch;
extern PID_Controller_t PID_Yaw;

void PID_Init(void);
// Hàm xay toán học PID
void PID_Compute(float current_roll, float current_pitch, float current_yaw, float dt);
// Bộ trộn động cơ
void Motor_Mixer(uint16_t base_throttle);

#endif