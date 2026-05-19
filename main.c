#include "01_fan_config.h"
#include "02_fan_register.h"
#include "03_fan_uart.h"     
#include "04_fan_LED_FND.h"  
#include "05_fan_motor.h"    
#include "06_fan_button.h"   

/* 시스템 전역 상태 변수 정의 */
volatile uint8_t is_running = 1;    // 시스템 동작 전원 플래그 (0: OFF, 1: ON)      
volatile uint8_t speed_state = 0;   // DC 모터 풍속 단계 플래그 (0 ~ 3단)
volatile uint8_t rotate_state = 0;  // 서보모터 스윙 모드 플래그 (0: OFF, 1: ON)

int main(void)
{
    /* 하드웨어 및 주변장치 초기화 시퀀스 */
    register_init();  // 기본 GPIO 입출력 방향 및 풀업 설정
    comm_init();      // UART0 및 외부 인터럽트(INT4), 타이머3 설정
    motor_init();     // 타이머1 기반 하드웨어 PWM 설정 (서보/DC 모터)
    visual_init();    // FND/LED 제어용 타이머0 소프트웨어 PWM 설정

    sei();            // 전역 인터럽트 활성화 (Global Interrupt Enable)
    
    uart0_print("\r\n[SYSTEM] ATmega128A Fan Project Boot Complete\r\n");


    /* 메인 폴링 루프 스케줄러 */
    while(1)
    {
        /* 1. 입력 태스크 스캔 (통신 및 GPIO 포트 폴링) */
        comm_process();    // IR 리모컨 수신 데이터 및 UART 커맨드 파싱
        button_process();  // 물리 버튼 입력 디바운싱 및 상태 천이 처리

        /* 2. 하드웨어 출력 및 제어 태스크 */
        set_fan_speed(speed_state); // 현재 속도 단계 기준 DC 모터 PWM 업데이트
        
        /* 3. 디스플레이 및 시각 피드백 태스크 */
        display_speed(speed_state); // 현재 속도 단계 FND 출력
        
        // 서보모터 스윙 위치 연산 및 연동 LED 잔상 알고리즘 업데이트
        update_led_sweep(update_motor_rotation()); 

        /* 시스템 안정화 및 루프 주기 제어 지연 */
        _delay_ms(15); 
    }
    
    return 0;
}