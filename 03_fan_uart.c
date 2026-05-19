#include "01_fan_config.h"
#include "03_fan_uart.h"


volatile uint32_t ir_code = 0;
volatile uint8_t ir_flag = 0;
volatile uint8_t saved_speed_state = 1;
volatile uint8_t saved_rotate_state = 0;



// -----------------------------------------------------------
// [1] 통신 및 타이머 하드웨어 초기화
// UART0 : PC터미널 통신(수신 인터럽트 활성화)
// Timer3 & INT4 : 리모컨(IR) 펄스의 하강 에지 시간차 측정
// -----------------------------------------------------------
void comm_init(void) {
    // 1. UART0 통신 초기화(보드 레이트 9600)
    UCSR0A |= (1 << U2X0); 
    // 💡 [수정됨] RXCIE0를 추가해서 수신(RX) 인터럽트의 '초인종'을 켰습니다!
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0); 
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00); 
    UBRR0H = 0; 
    UBRR0L = 207; 

    // 2. Timer3 초기화 (리모컨 시간 측정용, 분주비 64)
    TCCR3B |= (1 << CS31) | (1 << CS30);

    // 3. INT4 초기화 (리모컨 신호 인터럽트 허용, 하강 에지)
    EICRB |= (1 << ISC41); 
    EIMSK |= (1 << INT4);  
}

void uart0_transmit(char data) { // UART0로 1바이트 전송
    while(!(UCSR0A & (1 << UDRE0))); // 송신 버퍼가 비워질 때까지 대기
    UDR0 = data; // 데이터 전송
}

void uart0_print(char *str) { // 문장을 받아서 PC의 터미널(UART0)로 출력
    while (*str) uart0_transmit(*str++); // HW는 1바이트(한 글자)씩 전송하므로, 문장을 한 글자씩 쪼개줌.
    // 예시) "Hello\r\n" -> 'H', 'e', 'l', 'l', 'o', '\r', '\n' 순서로 전송.. NULL문자 만날 때 까지 반복.
}

// -----------------------------------------------------------
// [2] 외부 인터럽트 (INT4): IR 리모컨 프로토콜 디코딩
// ISR은 인터럽트 서비스 루틴의 약자. 인터럽트가 발생했을 때 자동으로 호출되는 함수입니다.
// -----------------------------------------------------------
ISR(INT4_vect) {
    static uint16_t last_time = 0; // 이전 펄스의 시간 기록 (TCNT3 값)
    uint16_t current_time = TCNT3;  // 현재 펄스의 시간 기록 (TCNT3 값)
    uint16_t pulse_width = current_time - last_time; // 펄스 폭 계산 (현재 시간 - 이전 시간)
    last_time = current_time; // 다음 인터럽트 때 사용할 수 있도록 현재 시간을 이전 시간으로 저장

    static uint8_t bit_cnt = 0; // 현재까지 읽은 비트 수 기록
    static uint32_t temp_code = 0; // 현재까지 읽은 비트로 구성된 임시 코드 저장 (최종적으로 ir_code에 저장될 값)

    if (pulse_width > 2000 && pulse_width < 5000) { // 시작 펄스 감지 (2ms ~ 5ms)
        bit_cnt = 0; // 비트 카운터 초기화
        temp_code = 0; // 임시 코드 초기화
    } 
    else if (pulse_width > 100 && pulse_width < 1000) {
        if (pulse_width > 400) { 
            temp_code |= (1UL << bit_cnt); 
        }
        bit_cnt++;
        
        if (bit_cnt == 32) {
            ir_code = temp_code;
            ir_flag = 1; 
            bit_cnt = 0;
        }
    }
}

// -----------------------------------------------------------
// [3] UART 수신 인터럽트 (PC 터미널에서 선풍기 제어)
// PC에서 숫자를 누르면 풍속 변경, P를 누르면 재생/정지 작동
// -----------------------------------------------------------
ISR(USART0_RX_vect) {
    char rx_data = UDR0;
    
    if (rx_data == '0') {
        speed_state = 0;
        is_running = 0;
        uart0_print("\r\n[UART] Fan OFF\r\n");
    }
    else if (rx_data >= '1' && rx_data <= '3') {
        speed_state = rx_data - '0'; // 문자를 숫자로 변환
        is_running = 1;
        uart0_print("\r\n[UART] Speed Changed\r\n");
    }
    else if (rx_data == 'r' || rx_data == 'R') {
        rotate_state ^= 1;
        uart0_print("\r\n[UART] Rotate Toggled\r\n");
    }
    else if (rx_data == 'p' || rx_data == 'P') {
        // PC에서 P를 누르면 리모컨의 재생/정지 버튼을 누른 것과 같은 효과를 줌
        ir_code = IR_BTN_PLAYPAUSE;
        ir_flag = 1; 
    }
}

// -----------------------------------------------------------
// [4] 메인 통신 제어 프로세스 (main.c 의 while 루프에서 1줄로 호출됨)
// 리더님이 요청하신 "이전 상태를 기억하는 재생/정지" 핵심 로직 포함!
// -----------------------------------------------------------
// ==========================================================
// 4. 메인 통신 제어 프로세스 
// ==========================================================
void comm_process(void) {
    char buffer[40];

    // 리모컨(또는 PC P키) 신호가 수신되었을 때만 작동
    if (ir_flag == 1) {
        sprintf(buffer, "IR Code: 0x%08lX\r\n", ir_code);
        uart0_print(buffer); // 어떤 코드가 들어왔는지 PC로 확인

        // 1) 재생 / 정지 버튼 (>||)
        if (ir_code == IR_BTN_PLAYPAUSE) {
            if (is_running == 1) {
                // [동작 중 -> 정지]
                saved_speed_state = speed_state;
                saved_rotate_state = rotate_state;
                
                speed_state = 0; 
                rotate_state = 0; 
                is_running = 0;
                uart0_print(">> System PAUSED\r\n");
            } else {
                // [정지 중 -> 동작] 
                speed_state = (saved_speed_state == 0) ? 1 : saved_speed_state;
                rotate_state = saved_rotate_state;
                is_running = 1;
                uart0_print(">> System RESUMED\r\n");
            }
        }
        // 2) 풍속 증가 버튼 (+) : 3단까지만 올라감
        else if (ir_code == IR_BTN_SPEED_UP) {
            if (speed_state < 3) {
                speed_state++;
                uart0_print(">> Speed UP\r\n");
            } else {
                uart0_print(">> Speed is MAX(3)\r\n");
            }
            is_running = (speed_state == 0) ? 0 : 1;
        }
        // 3) 풍속 감소 버튼 (-) : 0단(정지)까지만 내려감
        else if (ir_code == IR_BTN_SPEED_DOWN) {
            if (speed_state > 0) {
                speed_state--;
                uart0_print(">> Speed DOWN\r\n");
            } else {
                uart0_print(">> Speed is MIN(0)\r\n");
            }
            is_running = (speed_state == 0) ? 0 : 1;
        }
        // 4) 회전 변경 버튼
        else if (ir_code == IR_BTN_ROTATE) {
            rotate_state ^= 1;
            uart0_print(">> Rotate Toggled\r\n");
        }

        ir_flag = 0; // 처리 완료 (깃발 내림)
    }
}