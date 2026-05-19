#ifndef FAN_VISUAL_H_
#define FAN_VISUAL_H_

#include "01_fan_config.h"

/* 디스플레이 및 시각 피드백 모듈 API */
void visual_init(void);                    // 타이머 및 GPIO 출력 초기화
void display_speed(uint8_t state);         // 풍속 상태 FND 출력
void update_led_sweep(uint16_t servo_pos); // 서보 위치 동기화 LED 잔상 알고리즘 연산

#endif /* FAN_VISUAL_H_ */