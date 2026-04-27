#include "pid_control.h"
#include "hal_timer_pwm.h" // Nhúng Timer 3 PWM vào để xuất lệnh cho 4 Motor

PID_Controller_t PID_Roll;
PID_Controller_t PID_Pitch;
PID_Controller_t PID_Yaw;

void PID_Init(void) {
    // Tạm thời để KP, KI, KD = 0. Sau này sẽ "tune" (cân chỉnh) từ từ
    PID_Roll.Kp = 1.2f; PID_Roll.Ki = 0.0f; PID_Roll.Kd = 0.0f;
    PID_Roll.integral = 0; PID_Roll.prev_error = 0; PID_Roll.out_max = 400.0f; 

    // Pitch giống hệt Roll vì Quadcopter thiết kế đối xứng
    PID_Pitch.Kp = 1.2f; PID_Pitch.Ki = 0.0f; PID_Pitch.Kd = 0.0f;
    PID_Pitch.integral = 0; PID_Pitch.prev_error = 0; PID_Pitch.out_max = 400.0f;

    // Yaw xoay ngang nên ma sát khác, thường cần Kp lớn hơn một chút
    PID_Yaw.Kp = 2.0f; PID_Yaw.Ki = 0.0f; PID_Yaw.Kd = 0.0f;
    PID_Yaw.integral = 0; PID_Yaw.prev_error = 0; PID_Yaw.out_max = 400.0f;
}

// Trái tim PID thực thụ
static float Calculate_Single_PID(PID_Controller_t *pid, float current_val, float dt) {
    // 1. Tính sai số (Lệch bao nhiêu độ so với kỳ vọng?)
    pid->error = pid->setpoint - current_val;
    
    // 2. Tích phân (Cộng dồn sai số theo thời gian)
    pid->integral += pid->error * dt;
    // Bẫy chống Wind-up (Chống tràn tích phân khi bị kẹt)
    if (pid->integral > pid->out_max) pid->integral = pid->out_max;
    else if (pid->integral < -pid->out_max) pid->integral = -pid->out_max;

    // 3. Đạo hàm (Tốc độ thay đổi của sai số)
    float derivative = (pid->error - pid->prev_error) / dt;
    pid->prev_error = pid->error;

    // 4. Tổng hợp lực (Gia giảm gia vị Kp, Ki, Kd)
    pid->out = (pid->Kp * pid->error) + (pid->Ki * pid->integral) + (pid->Kd * derivative);
    
    // Cắt ngọn lực đẩy nếu vượt trần an toàn
    if (pid->out > pid->out_max) pid->out = pid->out_max;
    else if (pid->out < -pid->out_max) pid->out = -pid->out_max;

    return pid->out;
}

void PID_Compute(float current_roll, float current_pitch, float current_yaw, float dt) {
    Calculate_Single_PID(&PID_Roll, current_roll, dt);
    Calculate_Single_PID(&PID_Pitch, current_pitch, dt);
    Calculate_Single_PID(&PID_Yaw, current_yaw, dt);
}

// BỘ TRỘN CÔNG SUẤT ĐỘNG CƠ (Sơ đồ chữ X)
void Motor_Mixer(uint16_t base_throttle) {
    // base_throttle: Ga cơ bản (0 -> 999) từ tay cầm điều khiển (Chưa làm nên ta sẽ giả lập sau)
    
    if (base_throttle < 50) {
        // An toàn tối cao: Ga quá nhỏ -> Tắt động cơ hoàn toàn, không chạy PID bù trừ gì hết
        HAL_TIM3_PWM_SetDuty(1, 0); HAL_TIM3_PWM_SetDuty(2, 0);
        HAL_TIM3_PWM_SetDuty(3, 0); HAL_TIM3_PWM_SetDuty(4, 0);
        return;
    }

    // Công thức trộn ma thuật cho Sơ đồ X:
    // M1 (Trái-Trước)    M2 (Phải-Trước)
    // M4 (Trái-Sau)      M3 (Phải-Sau)
    
    int16_t m1 = base_throttle + PID_Pitch.out + PID_Roll.out - PID_Yaw.out;
    int16_t m2 = base_throttle + PID_Pitch.out - PID_Roll.out + PID_Yaw.out;
    int16_t m3 = base_throttle - PID_Pitch.out - PID_Roll.out - PID_Yaw.out;
    int16_t m4 = base_throttle - PID_Pitch.out + PID_Roll.out + PID_Yaw.out;

    // Khóa an toàn 2 đầu (Không cho phép PWM vọt qua 999 hoặc lủng đáy dưới 0)
    if (m1 > 999) m1 = 999; if (m1 < 0) m1 = 0;
    if (m2 > 999) m2 = 999; if (m2 < 0) m2 = 0;
    if (m3 > 999) m3 = 999; if (m3 < 0) m3 = 0;
    if (m4 > 999) m4 = 999; if (m4 < 0) m4 = 0;

    // Xuất lệnh xuống 4 cánh quạt
    HAL_TIM3_PWM_SetDuty(1, m1);
    HAL_TIM3_PWM_SetDuty(2, m2);
    HAL_TIM3_PWM_SetDuty(3, m3);
    HAL_TIM3_PWM_SetDuty(4, m4);
}