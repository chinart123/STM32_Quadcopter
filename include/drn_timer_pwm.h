#ifndef DRN_TIMER_PWM_H
#define DRN_TIMER_PWM_H

#include <stdint.h>

void DRN_Timer_PWM_Init(void);
// Nhận vào channel_id (1-4) và phần trăm ga (0.0 - 100.0)
void DRN_Timer_PWM_SetDuty(uint8_t channel_id, float percent); 

#endif