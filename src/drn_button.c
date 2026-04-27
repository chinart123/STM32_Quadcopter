#include "drn_button.h"
#include "drn_main_board_choose.h"

#ifdef STM32F103C8T6
    #include "stm32f10x.h"
#elif defined(ESP32_S3_SUPERMINI)
    // #include "driver/gpio.h"
#endif

DRN_Button_Context drn_btn_PA0 = {1, 0, 0, 0, BTN_EVENT_NONE, 0, 500}; 
DRN_Button_Context drn_btn_PA1 = {1, 0, 0, 0, BTN_EVENT_NONE, 0, 200}; 

void DRN_Button_Init(void) {
#ifdef STM32F103C8T6
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    GPIOA->CRL &= ~((0xF << 0) | (0xF << 4)); 
    GPIOA->CRL |=  ((0x8 << 0) | (0x8 << 4)); 
    GPIOA->ODR |= (1 << 0) | (1 << 1);    
#elif defined(ESP32_S3_SUPERMINI)
    // Code ESP32 GPIO Init...
#endif
}

void DRN_Button_State_Hardware_Scan(void) {
#ifdef STM32F103C8T6
    drn_btn_PA0.pin_state = (GPIOA->IDR & (1UL << 0)) ? 1 : 0;
    drn_btn_PA1.pin_state = (GPIOA->IDR & (1UL << 1)) ? 1 : 0;
#elif defined(ESP32_S3_SUPERMINI)
    drn_btn_PA0.pin_state = 1;
    drn_btn_PA1.pin_state = 1;
#endif
}

void DRN_Button_FSM_Process(DRN_Button_Context *btn, uint32_t current_tick) {
    if (current_tick - btn->last_update_tick >= 10) {
        btn->last_update_tick = current_tick; 
        
        if (btn->pin_state == 0) { 
            btn->release_duration = 0;
            if (btn->press_duration < 60000) btn->press_duration++;

            if (btn->press_duration == btn->hold_target) { 
                btn->event_code = BTN_EVENT_HOLD;
                btn->click_count = 0;
            }
        } 
        else { 
            if (btn->press_duration > 0) { 
                if (btn->press_duration > BTN_TICK_DEBOUNCE && btn->press_duration < btn->hold_target) {
                    btn->click_count++;
                }
                btn->press_duration = 0; 
            }

            if (btn->click_count > 0) {
                if (btn->release_duration < 60000) btn->release_duration++;

                if (btn->click_count == 2) {
                    btn->event_code = BTN_EVENT_DOUBLE_CLICK;
                    btn->click_count = 0;
                    btn->release_duration = 0;
                } 
                else if (btn->release_duration > BTN_TICK_DOUBLE_GAP) { 
                    if (btn->click_count == 1) {
                        btn->event_code = BTN_EVENT_SINGLE_CLICK;
                    }
                    btn->click_count = 0;
                    btn->release_duration = 0;
                }
            } else {
                btn->release_duration = 0; 
            }
        }
    }
}