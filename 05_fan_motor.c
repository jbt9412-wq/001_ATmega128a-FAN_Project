#include "01_fan_config.h"

/* 풍속 단계별 DC 모터 PWM 매핑 테이블 (0 ~ 3단) */
static uint16_t fanSpeedTable[] = {0, 2500, 4000, 4999};

/* 서보모터 위치 및 방향 제어 내부 변수 */
static uint16_t local_servo_pos = 375; // 초기 위치: 중앙 제어값 (1.5ms 듀티)
static int8_t local_servo_dir = 1;     // 이동 방향 (1: 정방향, -1: 역방향)

/* 타이머1 기반 하드웨어 PWM 및 GPIO 초기화 (서보/DC 모터 제어용) */
void motor_init(void) {
    // PB5(OC1A): 서보모터 PWM 출력, PB6(OC1B): DC모터 PWM 출력 설정
    DDRB |= (1 << PB5) | (1 << PB6);

    // Timer1 설정: Fast PWM (Mode 14), 비반전 모드(Non-Inverting), 분주비 64
    TCCR1A |= (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
    TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS11) | (1 << CS10); 
    
    ICR1 = 4999; // PWM 주기 설정 (50Hz, 20ms 프레임)
    
    OCR1A = 375; // 서보모터 초기 위치 설정 (중앙 정렬)
    OCR1B = 0;   // DC 모터 초기 상태 설정 (정지)
}

/* 풍속 상태 제어 및 PWM 듀티비 업데이트 */
void set_fan_speed(uint8_t state) {
    if (state > 3) state = 0; // 예외 처리 (범위 초과 시 정지)
    
    OCR1B = fanSpeedTable[state];

    // 0단(정지) 진입 시 회전 모드 해제 및 서보모터 원점(중앙) 복귀
    if (state == 0) {
        rotate_state = 0;     
        local_servo_pos = 375; 
        OCR1A = 375;          
    }
}

/* 스윙(회전) 모드 위치 업데이트 알고리즘 (주기적 호출) */
uint16_t update_motor_rotation(void) {
    if (rotate_state == 1) {
        local_servo_pos += local_servo_dir;
        
        // 구동 범위 소프트웨어 한계값 검증 (상한: 500, 하한: 250) 및 방향 전환
        if (local_servo_pos >= 500) local_servo_dir = -1;      
        else if (local_servo_pos <= 250) local_servo_dir = 1;  
        
        OCR1A = local_servo_pos; 
    }
    
    // 디스플레이(LED Bar) 동기화를 위한 현재 서보 위치 반환
    return local_servo_pos;
}