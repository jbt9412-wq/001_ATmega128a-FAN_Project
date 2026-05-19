#ifndef FAN_CONFIG_H_
#define FAN_CONFIG_H_

/* MCU 클럭 주파수 설정 */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* 표준 및 AVR 라이브러리 */
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>

/* 프로젝트 모듈 헤더 */
#include "02_fan_register.h"  // 핀 맵핑 및 하드웨어 설정
#include "03_fan_uart.h"      // UART 통신 인터페이스
#include "04_fan_LED_FND.h"   // 디스플레이(LED/FND) 제어
#include "05_fan_motor.h"     // PWM 모터 제어
#include "06_fan_button.h"    // 버튼 외부 인터럽트 입력

/* 시스템 전역 상태 플래그 */
extern volatile uint8_t is_running;    // 시스템 전원 상태 (0: 정지, 1: 동작)
extern volatile uint8_t speed_state;   // 현재 풍속 단계 (0 ~ 3단)
extern volatile uint8_t rotate_state;  // 회전 모드 상태 (0: 정지, 1: 회전)

/* 외부 모듈 변수 참조 */
extern volatile uint8_t brightness[8]; // LED 잔상 효과 제어용 배열

/* 모듈 초기화 함수 */
void motor_init(void);
void visual_init(void);

#endif /* FAN_CONFIG_H_ */