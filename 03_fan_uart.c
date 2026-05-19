#include "01_fan_config.h"
#include "03_fan_uart.h"

volatile uint32_t ir_code = 0;
volatile uint8_t ir_flag = 0;
volatile uint8_t saved_speed_state = 1;
volatile uint8_t saved_rotate_state = 0;

/* 통신 및 타이머 초기화 (UART0, Timer3, INT4) */
void comm_init(void) {
    // UART0: 9600 bps, 2배속 모드, Rx/Tx 활성화, Rx 인터럽트 활성화
    UCSR0A |= (1 << U2X0); 
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0); 
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00); 
    UBRR0H = 0; 
    UBRR0L = 207; 

    // Timer3: 정밀 시간 측정용 설정 (분주비 64)
    TCCR3B |= (1 << CS31) | (1 << CS30);

    // INT4: IR 수신 신호 입력 (하강 에지 트리거)
    EICRB |= (1 << ISC41); 
    EIMSK |= (1 << INT4);  
}

/* UART0 1바이트 송신 */
void uart0_transmit(char data) {
    while(!(UCSR0A & (1 << UDRE0))); 
    UDR0 = data; 
}

/* UART0 문자열 송신 */
void uart0_print(char *str) {
    while (*str) uart0_transmit(*str++); 
}

/* 외부 인터럽트 4번: NEC IR 프로토콜 디코딩 */
ISR(INT4_vect) {
    static uint16_t last_time = 0;
    uint16_t current_time = TCNT3;
    uint16_t pulse_width = current_time - last_time;
    last_time = current_time;

    static uint8_t bit_cnt = 0;
    static uint32_t temp_code = 0;

    // Leader Code 감지 (입력 펄스 폭 기준 약 2.25ms ~ 4.5ms 범위 검증)
    if (pulse_width > 2000 && pulse_width < 5000) {
        bit_cnt = 0;
        temp_code = 0;
    } 
    // Data Bit 판별 (Logic 0: 약 560us, Logic 1: 약 2.25ms)
    else if (pulse_width > 100 && pulse_width < 1000) {
        if (pulse_width > 400) { 
            temp_code |= (1UL << bit_cnt); 
        }
        bit_cnt++;
        
        // 32비트 프레임 수신 완료 검증
        if (bit_cnt == 32) {
            ir_code = temp_code;
            ir_flag = 1; 
            bit_cnt = 0;
        }
    }
}

/* UART0 수신 인터럽트 제어 서비스 */
ISR(USART0_RX_vect) {
    char rx_data = UDR0;
    
    if (rx_data == '0') {
        speed_state = 0;
        is_running = 0;
        uart0_print("\r\n[UART] Fan OFF\r\n");
    }
    else if (rx_data >= '1' && rx_data <= '3') {
        speed_state = rx_data - '0'; 
        is_running = 1;
        uart0_print("\r\n[UART] Speed Changed\r\n");
    }
    else if (rx_data == 'r' || rx_data == 'R') {
        rotate_state ^= 1;
        uart0_print("\r\n[UART] Rotate Toggled\r\n");
    }
    else if (rx_data == 'p' || rx_data == 'P') {
        ir_code = IR_BTN_PLAYPAUSE;
        ir_flag = 1; 
    }
}

/* 수신 데이터 처리 및 시스템 제어 로직 */
void comm_process(void) {
    char buffer[40];

    if (ir_flag == 1) {
        sprintf(buffer, "IR Code: 0x%08lX\r\n", ir_code);
        uart0_print(buffer); 

        // 재생/정지 제어 및 전 동작 상태 복원 로직
        if (ir_code == IR_BTN_PLAYPAUSE) {
            if (is_running == 1) {
                saved_speed_state = speed_state;
                saved_rotate_state = rotate_state;
                
                speed_state = 0; 
                rotate_state = 0; 
                is_running = 0;
                uart0_print(">> System PAUSED\r\n");
            } else {
                speed_state = (saved_speed_state == 0) ? 1 : saved_speed_state;
                rotate_state = saved_rotate_state;
                is_running = 1;
                uart0_print(">> System RESUMED\r\n");
            }
        }
        // 풍속 증가 제어 (최대 3단 상한 설정)
        else if (ir_code == IR_BTN_SPEED_UP) {
            if (speed_state < 3) {
                speed_state++;
                uart0_print(">> Speed UP\r\n");
            } else {
                uart0_print(">> Speed is MAX(3)\r\n");
            }
            is_running = (speed_state == 0) ? 0 : 1;
        }
        // 풍속 감소 제어 (최하 0단 하한 설정)
        else if (ir_code == IR_BTN_SPEED_DOWN) {
            if (speed_state > 0) {
                speed_state--;
                uart0_print(">> Speed DOWN\r\n");
            } else {
                uart0_print(">> Speed is MIN(0)\r\n");
            }
            is_running = (speed_state == 0) ? 0 : 1;
        }
        // 회전 모드 토글 제어
        else if (ir_code == IR_BTN_ROTATE) {
            rotate_state ^= 1;
            uart0_print(">> Rotate Toggled\r\n");
        }

        ir_flag = 0; 
    }
}