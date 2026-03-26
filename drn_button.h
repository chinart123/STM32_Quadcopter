#ifndef DRN_BUTTON_H
#define DRN_BUTTON_H

#include <stdint.h>

void DRN_Button_Init(void);

// Trả về 1 nếu nút ĐANG BỊ NHẤN (kéo xuống GND), 0 nếu đang nhả
uint8_t DRN_Button_Read_PA0(void);
uint8_t DRN_Button_Read_PA1(void);

#endif