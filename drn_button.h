#ifndef DRN_BUTTON_H
#define DRN_BUTTON_H

#include <stdint.h>
// Bỏ include stm32f10x.h ở đây để .h trong sạch, mang vào .c

typedef enum {
    BTN_TICK_DEBOUNCE   = 3,     
    BTN_TICK_DOUBLE_GAP = 30     
} DRN_ButtonTiming_t;

typedef enum {
    BTN_EVENT_NONE = 0,
    BTN_EVENT_SINGLE_CLICK,
    BTN_EVENT_DOUBLE_CLICK,
    BTN_EVENT_HOLD
} DRN_ButtonEvent_t;

typedef struct {
    uint8_t             pin_state;        
    uint32_t            press_duration;   
    uint32_t            release_duration; 
    uint8_t             click_count;      
    DRN_ButtonEvent_t   event_code;       
    uint32_t            last_update_tick; 
    uint32_t            hold_target;      
} DRN_Button_Context;

extern DRN_Button_Context drn_btn_PA0;
extern DRN_Button_Context drn_btn_PA1;

void DRN_Button_Init(void);
void DRN_Button_State_Hardware_Scan(void);
void DRN_Button_FSM_Process(DRN_Button_Context *btn, uint32_t current_tick);

#endif