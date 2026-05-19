#include "01_fan_config.h"
#include "04_fan_LED_FND.h"

/* FND 공통 캐소드 세그먼트 패턴 테이블 (0 ~ 3) */
static uint8_t fndNumber[] = {0x3F, 0x06, 0x5B, 0x4F};

volatile uint8_t brightness[8] = {0};

/* 시각 피드백 모듈 초기화 (Timer0 인터럽트 설정 포함) */
void visual_init(void) {
    DDRC = 0xFF; // PORTC: FND 출력 설정
    DDRD = 0xFF; // PORTD: LED 바 출력 설정

    // Timer0: 소프트웨어 PWM 구현용 타이머 (분주비 8, 오버플로우 인터럽트 활성화)
    TCCR0 |= (1 << CS01); 
    TIMSK |= (1 << TOIE0); 
}

/* 현재 풍속 단계 FND 출력 */
void display_speed(uint8_t state) {
    if (state > 3) state = 0; // 예외 처리 (범위 초과 시 0단으로 고정)
    PORTC = fndNumber[state]; 
}

/* 서보모터 위치 기반 LED 잔상 효과 업데이트 알고리즘 */
void update_led_sweep(uint16_t servo_pos) {
    // 서보모터 구동 범위(250~500) 내 8개 LED의 맵핑 좌표축
    int16_t led_pos[8] = {250, 286, 321, 357, 393, 429, 464, 500};
    int16_t spread = 50; // LED 인접 영역 번짐 범위 임계값

    if (rotate_state == 1) { 
        for(int i = 0; i < 8; i++) {
            // 현재 서보 위치와 각 LED 포지션 간의 절대 거리 계산
            int16_t diff = abs((int16_t)servo_pos - led_pos[i]); 
            
            // 임계값 이내일 경우 거리에 반비례하여 밝기 할당 (0 ~ 100%)
            if (diff < spread) {
                brightness[i] = 100 - (diff * 100 / spread);
            } else {
                brightness[i] = 0;
            }
        }
    } else { 
        for(int i = 0; i < 8; i++) brightness[i] = 0;
    }
}

/* Timer0 오버플로우 ISR: 8채널 독립 소프트웨어 PWM 제어 (해상도: 100분주) */
ISR(TIMER0_OVF_vect) 
{
    static uint8_t soft_pwm_cnt = 0;
    uint8_t out_portd = 0;

    soft_pwm_cnt++;
    if (soft_pwm_cnt >= 100) soft_pwm_cnt = 0; 
    
    // 듀티비 정밀 비교를 통한 PORTD(PD0~PD7) 개별 비트 마스킹 및 출력 제어
    if (brightness[0] > soft_pwm_cnt) out_portd |= (1 << PD0);
    if (brightness[1] > soft_pwm_cnt) out_portd |= (1 << PD1);
    if (brightness[2] > soft_pwm_cnt) out_portd |= (1 << PD2);
    if (brightness[3] > soft_pwm_cnt) out_portd |= (1 << PD3);
    if (brightness[4] > soft_pwm_cnt) out_portd |= (1 << PD4);
    if (brightness[5] > soft_pwm_cnt) out_portd |= (1 << PD5);
    if (brightness[6] > soft_pwm_cnt) out_portd |= (1 << PD6);
    if (brightness[7] > soft_pwm_cnt) out_portd |= (1 << PD7);

    PORTD = out_portd; 
}