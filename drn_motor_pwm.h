#ifndef DRN_MOTOR_PWM_H
#define DRN_MOTOR_PWM_H

#include <stdint.h>
#include "drn_button.h" 

// =========================================================
// ĐỊNH NGHĨA CÁC TRẠNG THÁI LOGIC CỦA HỆ THỐNG (Đem ra .h để dễ đọc)
// =========================================================
#define CMD_GATE_OPEN       0x01  // Mở cổng (Arm)
#define CMD_GATE_CLOSE      0x04  // Đóng cổng (Disarm)

#define CMD_RAMP_0_TO_50    0x02  // Tăng ga từ 0 lên 50%
#define CMD_RAMP_50_TO_100  0x05  // Tăng ga từ 50 lên 100%
#define CMD_RAMP_100_TO_0   0x03  // Giảm ga từ 100 về 0%

// THÊM MỚI: Các mốc 20%, 70%, 90%
#define CMD_RAMP_TO_20      0x06  // Tăng ga lên 20%
#define CMD_RAMP_TO_70      0x07  // Tăng ga lên 70%
#define CMD_RAMP_TO_90      0x08  // Tăng ga lên 90%

// =========================================================
// PHƠI BÀY CÁC BIẾN TEST ĐỂ XEM TRONG WATCH WINDOW
// =========================================================
extern float xx_pwm_motor_1; 
extern float xx_pwm_step; 
extern uint8_t xx_gate_state; 
extern uint8_t xx_ramp_state; 

// =========================================================
// KHAI BÁO HÀM
// =========================================================
void DRN_Motor_PWM_Init(void);
void DRN_Motor_Update_Logic(DRN_ButtonEvent_t arm_event, DRN_ButtonEvent_t mode_event);
void DRN_Motor_Run_Task(uint32_t current_time);

#endif