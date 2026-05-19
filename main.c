#include "01_fan_config.h" // 핀 설정, 상수 정의
#include "02_fan_register.h" // 레지스터 초기화 함수 선언
#include "03_fan_uart.h"     // UART 통신 관련 함수 선언
#include "04_fan_LED_FND.h"  // FND, LED 제어 관련 함수 선언
#include "05_fan_motor.h"    // 모터 제어 관련 함수 선언
#include "06_fan_button.h"   // 버튼 입력 처리 관련 함수 선언

// [시스템에서 제어할 3가지 상태]
volatile uint8_t is_running = 1;   // 시스템 전체의 ON/OFF 상태 (0: OFF, 1: ON)      
volatile uint8_t speed_state = 0;  // 풍속 상태 (0: 정지, 1: 저속, 2: 중속, 3: 고속) - 버튼 입력에 따라 변경
volatile uint8_t rotate_state = 0;  // 회전 방향 상태 (0: 시계방향, 1: 반시계방향) - UART 명령에 따라 변경

int main(void)
{
    // 1. 초기화 구역태
    register_init();  // 레지스터 초기화 (핀 모드 설정, 인터럽트 설정 등)
    comm_init();      // UART 통신 초기화
    motor_init();     // 모터 초기화 (PWM 설정 등)
    visual_init();    // 시각 출력 초기화 (FND, LED 핀 설정 및 타이머0 초기화)

    sei();  // 전역 인터럽트 허용
    uart0_print("\r\n=== Fan Project Boot: Ultimate Diet ===\r\n"); // 부팅 메시지 출력


    while(1)
    {
        // --- [STEP 1] 입력 감시 ---
        comm_process();    // UART 명령 처리 (회전 방향 변경 등)
        button_process();  // 버튼 입력 처리 (풍속 상태 변경)

        // --- [STEP 2] 모터 제어 ---
        set_fan_speed(speed_state); // 현재 풍속 상태
        
        // --- [STEP 3] 시각 출력  ---
        display_speed(speed_state); // FND에 현재 풍속 표시              
        update_led_sweep(update_motor_rotation()); // 모터 회전 상태에 따른 LED 잔상 업데이트

        _delay_ms(15); // 시스템 안정화를 위한 딜레이 (15ms)
    }
    return 0;
}