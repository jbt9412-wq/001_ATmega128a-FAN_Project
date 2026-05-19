#include "01_fan_config.h"
#include "06_fan_button.h"

/* 버튼 직전 상태 저장용 내부 정적 변수 (Initial State: Pull-up) */
static uint8_t prev_btn0 = 1; 
static uint8_t prev_btn1 = 1; 

/* GPIO 폴링 기반 버튼 입력 처리 및 디바운싱 알고리즘 */
void button_process(void) {
    // PINA 레지스터로부터 PA0, PA1 현재 입력 상태 핀 스캔
    uint8_t curr_btn0 = (PINA & (1 << PA0)) >> PA0;
    uint8_t curr_btn1 = (PINA & (1 << PA1)) >> PA1;

    /* PA0: 풍속 제어 버튼 처리 (Falling Edge 감지 및 디바운싱) */
    if (curr_btn0 == 0 && prev_btn0 == 1) {
        _delay_ms(30); // 소프트웨어 디바운싱 지연 (채터링 노이즈 제거)
        if (((PINA & (1 << PA0)) >> PA0) == 0) {
            speed_state++;
            if (speed_state > 3) speed_state = 0; 
            is_running = (speed_state == 0) ? 0 : 1;
        }
    }
    prev_btn0 = curr_btn0; // 버튼 0 전차 상태 업데이트

    /* PA1: 스윙 모드 버튼 처리 (Falling Edge 감지 및 디바운싱) */
    if (curr_btn1 == 0 && prev_btn1 == 1) {
        _delay_ms(30); // 소프트웨어 디바운싱 지연 (채터링 노이즈 제거)
        if (((PINA & (1 << PA1)) >> PA1) == 0) {
            rotate_state ^= 1; 
        }
    }
    prev_btn1 = curr_btn1; // 버튼 1 전차 상태 업데이트
}