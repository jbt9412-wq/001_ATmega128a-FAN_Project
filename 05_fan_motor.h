#ifndef FAN_MOTOR_H_
#define FAN_MOTOR_H_

#include "01_fan_config.h" 

/* PWM 모터 제어 모듈 API */
void motor_init(void);              // 타이머1 및 PWM 출력 초기화
void set_fan_speed(uint8_t state);  // 풍속 상태 제어 및 DC 모터 PWM 업데이트
uint16_t update_motor_rotation(void); // 스윙 모드 위치 업데이트 및 현재 위치 반환

#endif /* FAN_MOTOR_H_ */