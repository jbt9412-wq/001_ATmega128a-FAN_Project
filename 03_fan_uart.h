#ifndef FAN_UART_H_
#define FAN_UART_H_

#include "01_fan_config.h"

/* NEC IR 프로토콜 커맨드 매크로 */
#define IR_BTN_SPEED_UP   0xEA15FF00  // 풍속 증가 (+)
#define IR_BTN_SPEED_DOWN 0xF807FF00  // 풍속 감소 (-)
#define IR_BTN_ROTATE     0xBF40FF00  // 회전 토글
#define IR_BTN_PLAYPAUSE  0xBC43FF00  // 재생/정지

/* 외부 참조 전역 변수 (IR/UART 통신 플래그 및 버퍼) */
extern volatile uint32_t ir_code;           // 수신된 32비트 IR 프레임 버퍼
extern volatile uint8_t ir_flag;            // IR 프레임 수신 완료 플래그 (0: 대기, 1: 완료)
extern volatile uint8_t saved_speed_state;  // 정지 직전 풍속 상태 백업 버퍼 (0 ~ 3단)
extern volatile uint8_t saved_rotate_state; // 정지 직전 회전 상태 백업 버퍼 (0: 정지, 1: 회전)

/* 통신 모듈 외부 공개 API */
void comm_init(void);        // UART0, Timer3, INT4 하드웨어 초기화
void comm_process(void);     // IR 수신 플래그 폴링 및 시스템 상태 제어 프로세스
void uart0_print(char *str); // PC 터미널 모니터링용 문자열 송신 인터페이스

#endif /* FAN_UART_H_ */